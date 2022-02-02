/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderbloomcomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "valuegpubuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"
#include "blurshader.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::RenderBloomComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderBloomComponent)
	RTTI_PROPERTY("PassCount",					&nap::RenderBloomComponent::mPassCount,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Kernel",						&nap::RenderBloomComponent::mKernel,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputTexture",				&nap::RenderBloomComponent::mInputTexture,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::RenderBloomComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderBloomComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


/**
 * Creates a model matrix based on the dimensions of the given target.
 */
static void computeModelMatrix(const nap::IRenderTarget& target, glm::mat4& outMatrix)
{
	// Transform to middle of target
	glm::ivec2 tex_size = target.getBufferSize();
	outMatrix = glm::translate(glm::mat4(), glm::vec3(
		tex_size.x / 2.0f,
		tex_size.y / 2.0f,
		0.0f));

	// Scale to fit target
	outMatrix = glm::scale(outMatrix, glm::vec3(tex_size.x, tex_size.y, 1.0f));
}


namespace nap
{
	RenderBloomComponentInstance::RenderBloomComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPlane(*entity.getCore())
	{ }


	bool RenderBloomComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		RenderBloomComponent* resource = getComponent<RenderBloomComponent>();

		// Verify pass count
		if (!errorState.check(resource->mPassCount > 0, "Property 'PassCount' must be higher than zero"))
			return false;

		// Get reference to input texture
		mInputTexture = resource->mInputTexture.get();

		// Initialize double-buffered bloom render targets
		for (int pass_idx = 0; pass_idx < resource->mPassCount; pass_idx++)
		{
			DoubleBufferedRenderTarget& bloom_target = mBloomRTs.emplace_back();

			for (int target_idx = 0; target_idx < 2; target_idx++)
			{
				auto tex = getEntityInstance()->getCore()->getResourceManager()->createObject<RenderTexture2D>();
				tex->mWidth = resource->mInputTexture->getWidth() / math::power<int>(2, pass_idx+1);
				tex->mHeight = resource->mInputTexture->getHeight() / math::power<int>(2, pass_idx+1);
				tex->mFill = resource->mInputTexture->mFill;
				tex->mFormat = resource->mInputTexture->mFormat;
				tex->mUsage = ETextureUsage::Static;
				if (!tex->init(errorState))
				{
					errorState.fail("%s: Failed to initialize internal render texture", tex->mID.c_str());
					return false;
				}

				auto target = getEntityInstance()->getCore()->getResourceManager()->createObject<RenderTarget>();
				target->mColorTexture = tex;
				target->mClearColor = resource->mInputTexture->mClearColor;
				target->mSampleShading = false;
				target->mRequestedSamples = ERasterizationSamples::One;
				if (!target->init(errorState))
				{
					errorState.fail("%s: Failed to initialize internal render target", target->mID.c_str());
					return false;
				}

				bloom_target[target_idx] = target;
			}
		}

		// Get render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();

		// Create material
		Material* blur_material = nullptr;
		switch (resource->mKernel)
		{
		case EBlurSamples::X5:
			blur_material = mRenderService->getOrCreateMaterial<Blur5x5Shader>(errorState);
			break;
		case EBlurSamples::X9:
			blur_material = mRenderService->getOrCreateMaterial<Blur9x9Shader>(errorState);
			break;
		default:
			errorState.fail("Unsupported blur shader");
			return false;
		}
		if (!errorState.check(blur_material != nullptr, "%s: unable to get or create blur material", resource->mID.c_str()))
			return false;

		// Create material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = blur_material;
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Now create a plane and initialize it.
		// The model matrix is computed on draw and used to scale the model to fit target bounds.
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		mPlane.mUsage = EMemoryUsage::Static;
		mPlane.mCullMode = ECullMode::Back;
		mPlane.mColumns = 1;
		mPlane.mRows = 1;
		
		if (!mPlane.init(errorState))
			return false;

		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in material: %s",
			this->mID.c_str(), uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get model matrix (required)
		mModelMatrixUniform = ensureUniform(uniform::modelMatrix, *mMVPStruct, errorState);
		if (mModelMatrixUniform == nullptr)
			return false;

		// Get projection matrix (required)
		mProjectMatrixUniform = ensureUniform(uniform::projectionMatrix, *mMVPStruct, errorState);
		if (mProjectMatrixUniform == nullptr) 
			return false;

		// Get view matrix (optional)
		utility::ErrorState view_error;
		mViewMatrixUniform = ensureUniform(uniform::viewMatrix, *mMVPStruct, view_error);

		// Get UBO struct
		UniformStructInstance* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", this->mID.c_str(), uniform::uboStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get direction uniform
		mDirectionUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::direction);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		// Get texture size uniform
		mTextureSizeUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::textureSize);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		// Get color texture sampler
		mColorTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::sampler::colorTexture);
		if (!errorState.check(mColorTextureSampler != nullptr, "Missing uniform sampler2D 'colorTexture'"))
			return false;

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mRenderService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		return true;
	}


	void RenderBloomComponentInstance::draw()
	{
		VkCommandBuffer command_buffer = mRenderService->getCurrentCommandBuffer();
		glm::mat4 identity_matrix = glm::mat4();

		// Set first input texture
		RenderTexture2D* input_texture = mInputTexture;

		for (auto& bloom_target : mBloomRTs)
		{
			glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)bloom_target[0]->getBufferSize().x, 0.0f, (float)bloom_target[0]->getBufferSize().y);

			// Horizontal
			mColorTextureSampler->setTexture(*input_texture);
			mDirectionUniform->setValue({ 1.0f, 0.0f });
			mTextureSizeUniform->setValue(bloom_target[0]->mColorTexture->getSize());

			bloom_target[0]->beginRendering();
			onDraw(*bloom_target[0], command_buffer, identity_matrix, proj_matrix);
			bloom_target[0]->endRendering();

			// Vertical
			mColorTextureSampler->setTexture(*bloom_target[0]->mColorTexture);
			mDirectionUniform->setValue({ 0.0f, 1.0f });

			bloom_target[1]->beginRendering();
			onDraw(*bloom_target[1], command_buffer, identity_matrix, proj_matrix);
			bloom_target[1]->endRendering();

			// Set output texture of current pass to input texture of subsequent pass
			input_texture = bloom_target[1]->mColorTexture.get();
		}
	}


	void RenderBloomComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{        
		// Update the model matrix so that the plane mesh is of the same size as the render target
		computeModelMatrix(renderTarget, mModelMatrix);
		mModelMatrixUniform->setValue(mModelMatrix);

		// Update matrices, projection and model are required
		mProjectMatrixUniform->setValue(projectionMatrix);

		// If view matrix exposed on shader, set it as well
		if (mViewMatrixUniform != nullptr)
			mViewMatrixUniform->setValue(viewMatrix);

		// Get valid descriptor set
		const DescriptorSet& descriptor_set = mMaterialInstance.update();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind buffers and draw
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}


	UniformMat4Instance* RenderBloomComponentInstance::ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error)
	{
		// Get matrix binding
		UniformMat4Instance* found_mat = mvpStruct.getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_mat != nullptr, "%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(), mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;

		return found_mat;
	}
}
