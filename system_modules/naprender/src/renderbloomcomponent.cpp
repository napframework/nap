/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderbloomcomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "gpubuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"
#include "blurshader.h"
#include "textureutils.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::RenderBloomComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderBloomComponent, "Applies a bloom effect to a texture")
	RTTI_PROPERTY("PassCount",					&nap::RenderBloomComponent::mPassCount,					nap::rtti::EPropertyMetaData::Default,	"Number of combined horizontal / vertical passes")
	RTTI_PROPERTY("Kernel",						&nap::RenderBloomComponent::mKernel,					nap::rtti::EPropertyMetaData::Default,	"The blur kernel")
	RTTI_PROPERTY("InputTexture",				&nap::RenderBloomComponent::mInputTexture,				nap::rtti::EPropertyMetaData::Required,	"The texture to apply the effect to")
	RTTI_PROPERTY("OutputTexture",				&nap::RenderBloomComponent::mOutputTexture,				nap::rtti::EPropertyMetaData::Required, "The texture the effect is applied to")
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
	auto tex_size = target.getBufferSize();
	auto tex_center = glm::vec3(tex_size.x * 0.5f, tex_size.y * 0.5f, 0.0f);
	outMatrix = glm::translate(glm::identity<glm::mat4>(), tex_center);

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
		auto* resource = getComponent<RenderBloomComponent>();

		// Verify pass count
		if (!errorState.check(resource->mPassCount > 0, "Property 'PassCount' must be higher than zero"))
			return false;

		// Get reference to input texture
		mInputTexture = resource->mInputTexture.get();
		mOutputTexture = resource->mOutputTexture.get();

		mBloomRTs.reserve(resource->mPassCount);
		mBloomTextures.reserve(resource->mPassCount);

		// Initialize double-buffered bloom render targets
		for (int pass_idx = 0; pass_idx < resource->mPassCount; pass_idx++)
		{
			auto& bloom_target = mBloomRTs.emplace_back();
			auto& bloom_texture = mBloomTextures.emplace_back();

			for (int target_idx = 0; target_idx < 2; target_idx++)
			{
				auto& tex = bloom_texture[target_idx];
				tex = std::make_unique<RenderTexture2D>(*getEntityInstance()->getCore());
				tex->mWidth = resource->mInputTexture->getWidth() / math::power<int>(2, pass_idx+1);
				tex->mHeight = resource->mInputTexture->getHeight() / math::power<int>(2, pass_idx+1);
				tex->mColorFormat = resource->mInputTexture->mColorFormat;
				tex->mUsage = Texture::EUsage::Static;
				if (!tex->init(errorState))
				{
					errorState.fail("%s: Failed to initialize internal render texture", tex->mID.c_str());
					return false;
				}

				auto& target = bloom_target[target_idx];
				target = std::make_unique<RenderTarget>(*getEntityInstance()->getCore());
				target->mColorTexture = tex.get();
				target->mClearColor = resource->mInputTexture->mClearColor;
				target->mSampleShading = false;
				target->mRequestedSamples = ERasterizationSamples::One;

				if (!target->init(errorState))
				{
					errorState.fail("%s: Failed to initialize internal render target", target->mID.c_str());
					return false;
				}
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
		case EBlurSamples::X13:
			blur_material = mRenderService->getOrCreateMaterial<Blur13x13Shader>(errorState);
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
		auto* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::blur::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to find uniform UBO struct: %s in material: %s", this->mID.c_str(), uniform::blur::uboStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get direction uniform
		mDirectionUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::blur::direction);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		// Get texture size uniform
		mTextureSizeUniform = ubo_struct->getOrCreateUniform<UniformVec2Instance>(uniform::blur::textureSize);
		if (!errorState.check(mDirectionUniform != nullptr, "Missing uniform vec2 'direction' in uniform UBO"))
			return false;

		// Get color texture sampler
		mColorTextureSampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::blur::sampler::colorTexture);
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
		const auto command_buffer = mRenderService->getCurrentCommandBuffer();
		const glm::mat4 identity_matrix = {};

		// Blit the input texture to the smaller size RT
		auto& initial_texture = *mBloomRTs.front().front()->mColorTexture;
		utility::blit(command_buffer, *mInputTexture, initial_texture);

		int pass_count = 0;
		for (auto& target : mBloomRTs)
		{
			const auto proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(
				0.0f, static_cast<float>(target.front()->getBufferSize().x),
				0.0f, static_cast<float>(target.front()->getBufferSize().y));

			// Horizontal
			mColorTextureSampler->setTexture(*target.front()->mColorTexture);
			mDirectionUniform->setValue({ 1.0f, 0.0f });
			mTextureSizeUniform->setValue(target.front()->mColorTexture->getSize());

			target.back()->beginRendering();
			onDraw(*target.back(), command_buffer, identity_matrix, proj_matrix);
			target.back()->endRendering();

			// Vertical
			mColorTextureSampler->setTexture(*target.back()->mColorTexture);
			mDirectionUniform->setValue({ 0.0f, 1.0f });

			target.front()->beginRendering();
			onDraw(*target.front(), command_buffer, identity_matrix, proj_matrix);
			target.front()->endRendering();

			// Blit the input texture to the smaller size RT
			if (pass_count+1 < mBloomRTs.size())
			{
				auto& blit_dst = *mBloomRTs[pass_count+1].front()->mColorTexture;
				utility::blit(command_buffer, *target.front()->mColorTexture, blit_dst);
				++pass_count;
			}
		}

		// Get a reference to the bloom result
		// The size of this texture equals { input_width / 2 ^ PassCount, input_height / 2 ^ PassCount }
		auto& final_texture = *mBloomRTs.back().front()->mColorTexture;

		// Blit to output, potentially resizing the texture another time
		utility::blit(command_buffer, final_texture, *mOutputTexture);
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
		const auto& descriptor_set = mMaterialInstance.update();

		// Gather draw info
		const auto& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		const auto& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		auto pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind buffers and draw
		const auto& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const auto& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const auto& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}


	UniformMat4Instance* RenderBloomComponentInstance::ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error)
	{
		// Get matrix binding
		auto* found_mat = mvpStruct.getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_mat != nullptr, "%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(), mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;

		return found_mat;
	}
}
