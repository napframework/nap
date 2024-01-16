/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderdofcomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"
#include "dofshader.h"
#include "textureutils.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::RenderDOFComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderDOFComponent)
	RTTI_PROPERTY("Camera",						&nap::RenderDOFComponent::mCamera,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PassCount",					&nap::RenderDOFComponent::mPassCount,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputTarget",				&nap::RenderDOFComponent::mInputTarget,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputTexture",				&nap::RenderDOFComponent::mOutputTexture,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Aperture",					&nap::RenderDOFComponent::mAperture,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FocusPower",					&nap::RenderDOFComponent::mFocusPower,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FocusDistance",				&nap::RenderDOFComponent::mFocusDistance,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::RenderDOFComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderDOFComponentInstance)
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
	RenderDOFComponentInstance::RenderDOFComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPlane(*entity.getCore())
	{ }


	bool RenderDOFComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		RenderDOFComponent* resource = getComponent<RenderDOFComponent>();
		mResource = resource;

		// Verify the input render target is available
		if (!errorState.check(resource->mInputTarget != nullptr, "InputTarget not set"))
			return false;

		// Verify the render target has a depth texture resource
		if (!errorState.check(resource->mInputTarget->hasDepthTexture(), "The specified InputTarget has no depth texture resource"))
			return false;

		// Verify pass count
		if (!errorState.check(resource->mPassCount > 0, "Property 'PassCount' must be higher than zero"))
			return false;

		// Get reference to input texture
		mInputTarget = resource->mInputTarget.get();
		mOutputTexture = resource->mOutputTexture.get();

		// Initialize double-buffered bloom render targets
		for (int pass_idx = 0; pass_idx < resource->mPassCount; pass_idx++)
		{
			DoubleBufferedRenderTarget& bloom_target = mBloomRTs.emplace_back();

			for (int target_idx = 0; target_idx < 2; target_idx++)
			{
				auto tex = getEntityInstance()->getCore()->getResourceManager()->createObject<RenderTexture2D>();
				tex->mWidth = resource->mInputTarget->getColorTexture().getWidth() / math::power<int>(2, pass_idx);
				tex->mHeight = resource->mInputTarget->getColorTexture().getHeight() / math::power<int>(2, pass_idx);
				tex->mColorFormat = resource->mInputTarget->getColorTexture().mColorFormat;
				tex->mUsage = Texture::EUsage::Static;
				if (!tex->init(errorState))
				{
					errorState.fail("%s: Failed to initialize internal render texture", tex->mID.c_str());
					return false;
				}

				auto target = getEntityInstance()->getCore()->getResourceManager()->createObject<RenderTarget>();
				target->mColorTexture = tex;
				target->mClearColor = resource->mInputTarget->getColorTexture().mClearColor;
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
		Material* dof_material = mRenderService->getOrCreateMaterial<DOFShader>(errorState);
		if (!errorState.check(dof_material != nullptr, "%s: unable to get or create blur material", resource->mID.c_str()))
			return false;

		// Create material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = dof_material;
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Now create a plane and initialize it.
		// The model matrix is computed on draw and used to scale the model to fit target bounds.
		mPlane.mSize = { 1.0f, 1.0f };
		mPlane.mPosition = { 0.0f, 0.0f };
		mPlane.mUsage = EMemoryUsage::Static;
		mPlane.mCullMode = ECullMode::None;
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
		UniformStructInstance* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::dof::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", this->mID.c_str(), uniform::dof::uboStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get direction uniform
		mDirectionUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::dof::direction);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		// Get texture size uniform
		mTextureSizeUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::dof::textureSize);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		mNearFarUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::dof::nearFar);
		if (!errorState.check(mNearFarUniform != nullptr, "Missing uniform vec2 'nearFar' in uniform UBO"))
			return false;

		mApertureUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::aperture);
		if (!errorState.check(mApertureUniform != nullptr, "Missing uniform float 'aperture' in uniform UBO"))
			return false;

		mFocusPowerUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::focusPower);
		if (!errorState.check(mFocusPowerUniform != nullptr, "Missing uniform float 'focusPower' in uniform UBO"))
			return false;

		mFocusDistanceUniform = ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::dof::focusDistance);
		if (!errorState.check(mFocusDistanceUniform != nullptr, "Missing uniform float 'focusDistance' in uniform UBO"))
			return false;

		// Get color texture sampler
		mColorTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::dof::sampler::colorTexture);
		if (!errorState.check(mColorTextureSampler != nullptr, "Missing uniform sampler2D 'colorTexture'"))
			return false;

		// Get depth texture sampler
		mDepthTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::dof::sampler::depthTexture);
		if (!errorState.check(mDepthTextureSampler != nullptr, "Missing uniform sampler2D 'depthTexture'"))
			return false;

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mRenderService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		return true;
	}


	void RenderDOFComponentInstance::draw()
	{
		// Indices into a double-buffered render target
		const uint TARGET_A = 0;
		const uint TARGET_B = 1;

		const VkCommandBuffer command_buffer = mRenderService->getCurrentCommandBuffer();

		auto& initial_texture = *mBloomRTs.front()[TARGET_A]->mColorTexture;

		// Blit the input texture to the smaller size RT
		utility::blit(command_buffer, mInputTarget->getColorTexture(), initial_texture);
		assert(mInputTarget->hasDepthTexture());

		int pass_count = 0;
		for (auto& bloom_target : mBloomRTs)
		{
			glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)bloom_target[0]->getBufferSize().x, 0.0f, (float)bloom_target[0]->getBufferSize().y);

			// Horizontal
			mColorTextureSampler->setTexture(*bloom_target[TARGET_A]->mColorTexture);
			mDepthTextureSampler->setTexture(mInputTarget->getDepthTexture());
			mDirectionUniform->setValue({ 1.0f, 0.0f });
			mTextureSizeUniform->setValue(bloom_target[TARGET_A]->mColorTexture->getSize());
			mNearFarUniform->setValue({ mCamera->getNearClippingPlane(), mCamera->getFarClippingPlane() });
			mApertureUniform->setValue(mResource->mAperture->mValue);
			mFocusPowerUniform->setValue(mResource->mFocusPower->mValue);
			mFocusDistanceUniform->setValue(mResource->mFocusDistance->mValue);

			bloom_target[TARGET_B]->beginRendering();
			onDraw(*bloom_target[TARGET_B], command_buffer, glm::identity<glm::mat4>(), proj_matrix);
			bloom_target[TARGET_B]->endRendering();

			// Vertical
			mColorTextureSampler->setTexture(*bloom_target[TARGET_B]->mColorTexture);
			mDirectionUniform->setValue({ 0.0f, 1.0f });

			bloom_target[TARGET_A]->beginRendering();
			onDraw(*bloom_target[TARGET_A], command_buffer, glm::identity<glm::mat4>(), proj_matrix);
			bloom_target[TARGET_A]->endRendering();

			// Blit the input texture to the smaller size RT
			if (pass_count+1 < mBloomRTs.size())
			{
				auto& blit_dst = *mBloomRTs[pass_count + 1][TARGET_A]->mColorTexture;
				utility::blit(command_buffer, *bloom_target[TARGET_A]->mColorTexture, blit_dst);
				++pass_count;
			}
		}

		// Get a reference to the bloom result
		// The size of this texture equals { input_width / 2 ^ PassCount, input_height / 2 ^ PassCount }
		auto& final_texture = *mBloomRTs.back()[TARGET_A]->mColorTexture;

		// Blit to output, potentially resizing the texture another time
		utility::blit(command_buffer, final_texture, *mOutputTexture);
	}


	void RenderDOFComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
		const MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		const GPUMesh& mesh = mesh_instance.getGPUMesh();

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


	UniformMat4Instance* RenderDOFComponentInstance::ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error)
	{
		// Get matrix binding
		UniformMat4Instance* found_mat = mvpStruct.getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_mat != nullptr, "%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(), mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;

		return found_mat;
	}
}
