/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendervideocomponent.h"
#include "videoshader.h"

// External Includes
#include <entity.h>
#include <orthocameracomponent.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <glm/gtc/matrix_transform.hpp>

// nap::rendervideototexturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderVideoComponent)
	RTTI_PROPERTY("OutputTexture",	&nap::RenderVideoComponent::mOutputTexture,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VideoPlayer",	&nap::RenderVideoComponent::mVideoPlayer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SampleShading",	&nap::RenderVideoComponent::mSampleShading,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",		&nap::RenderVideoComponent::mRequestedSamples,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor",		&nap::RenderVideoComponent::mClearColor,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::rendervideototexturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderVideoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
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


	RenderVideoComponentInstance::RenderVideoComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mTarget(*entity.getCore()),
		mPlane(*entity.getCore())	{ }


	bool RenderVideoComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderVideoComponent* resource = getComponent<RenderVideoComponent>();

		// Extract player
		mPlayer = resource->mVideoPlayer.get();
		if (!errorState.check(mPlayer != nullptr, "%s: no video player", resource->mID.c_str()))
			return false;

		// Extract output texture to render to and make sure format is correct
		mOutputTexture = resource->mOutputTexture.get();
		if (!errorState.check(mOutputTexture != nullptr, "%s: no output texture", resource->mID.c_str()))
			return false;
		if (!errorState.check(mOutputTexture->mFormat == RenderTexture2D::EFormat::RGBA8, "%s: output texture color format is not RGBA8", resource->mID.c_str()))
			return false;

		// Setup render target and initialize
		mTarget.mClearColor = glm::vec4(resource->mClearColor.convert<RGBColorFloat>().toVec3(), 1.0f);
		mTarget.mColorTexture  = resource->mOutputTexture;
		mTarget.mSampleShading = resource->mSampleShading;
		mTarget.mRequestedSamples = resource->mRequestedSamples;
		if (!mTarget.init(errorState))
			return false;

		// Now create a plane and initialize it
		// The plane is positioned on update based on current texture output size
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		mPlane.mCullMode = ECullMode::Back;
		mPlane.mUsage = EMeshDataUsage::Static;
		mPlane.mColumns = 1;
		mPlane.mRows = 1;

		if (!mPlane.init(errorState))
			return false;

		// Extract render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mRenderService != nullptr);

		// Get video material
		Material* video_material = mRenderService->getOrCreateMaterial<VideoShader>(errorState);
		if (!errorState.check(video_material != nullptr, "%s: unable to get or create video material", resource->mID.c_str()))
			return false;

		// Create resource for the video material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial  = video_material;

		// Initialize video material instance, used for rendering video
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in material: %s",
			this->mID.c_str(), uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get all matrices
		mModelMatrixUniform = ensureUniform(uniform::modelMatrix, errorState);
		mProjectMatrixUniform = ensureUniform(uniform::projectionMatrix, errorState);
		mViewMatrixUniform = ensureUniform(uniform::viewMatrix, errorState);

		if (mModelMatrixUniform == nullptr || mProjectMatrixUniform == nullptr || mViewMatrixUniform == nullptr)
			return false;

		// Get sampler inputs to update from video material
		mYSampler = ensureSampler(uniform::video::YSampler, errorState);
		mUSampler = ensureSampler(uniform::video::USampler, errorState);
		mVSampler = ensureSampler(uniform::video::VSampler, errorState);

		if (mYSampler == nullptr || mUSampler == nullptr || mVSampler == nullptr)
			return false;

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mRenderService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Listen to video selection changes
		mPlayer->VideoChanged.connect(mVideoChangedSlot);

		// Update textures on initialization
		videoChanged(*mPlayer);

		return true;
	}


	bool RenderVideoComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	nap::Texture2D& RenderVideoComponentInstance::getOutputTexture()
	{
		return mTarget.getColorTexture();
	}


	void RenderVideoComponentInstance::draw()
	{
		// Get current command buffer, should be headless.
		VkCommandBuffer command_buffer = mRenderService->getCurrentCommandBuffer();

		// Create orthographic projection matrix
		glm::ivec2 size = mTarget.getBufferSize();

		// Create projection matrix
		glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)size.x, 0.0f, (float)size.y);

		// Call on draw
		mTarget.beginRendering();
		onDraw(mTarget, command_buffer, glm::mat4(), proj_matrix);
		mTarget.endRendering();
	}


	void RenderVideoComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Update the model matrix so that the plane mesh is of the same size as the render target
		computeModelMatrix(renderTarget, mModelMatrix);
		mModelMatrixUniform->setValue(mModelMatrix);

		// Update matrices, projection and model are required
		mProjectMatrixUniform->setValue(projectionMatrix);
		mViewMatrixUniform->setValue(viewMatrix);

		// Get valid descriptor set
		VkDescriptorSet descriptor_set = mMaterialInstance.update();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

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


	nap::UniformMat4Instance* RenderVideoComponentInstance::ensureUniform(const std::string& uniformName, utility::ErrorState& error)
	{
		assert(mMVPStruct != nullptr);
		UniformMat4Instance* found_uniform = mMVPStruct->getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_uniform != nullptr,
			"%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;
		return found_uniform;
	}


	nap::Sampler2DInstance* RenderVideoComponentInstance::ensureSampler(const std::string& samplerName, utility::ErrorState& error)
	{
		Sampler2DInstance* found_sampler = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(samplerName);
		if (!error.check(found_sampler != nullptr,
			"%s: unable to find sampler: %s in material: %s", this->mID.c_str(), samplerName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;
		return found_sampler;
	}

	
	void RenderVideoComponentInstance::videoChanged(VideoPlayer& player)
	{
		mYSampler->setTexture(player.getYTexture());
		mUSampler->setTexture(player.getUTexture());
		mVSampler->setTexture(player.getVTexture());
	}
}