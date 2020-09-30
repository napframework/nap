/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendertotexturecomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "indexbuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::rendertotexturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderToTextureComponent)
	RTTI_PROPERTY("OutputTexture",				&nap::RenderToTextureComponent::mOutputTexture,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",			&nap::RenderToTextureComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearColor",					&nap::RenderToTextureComponent::mClearColor,				nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::rendertotexturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderToTextureComponentInstance)
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
	RenderToTextureComponentInstance::RenderToTextureComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mTarget(*entity.getCore()),
        mPlane(*entity.getCore())
	{ }


	bool RenderToTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderToTextureComponent* resource = getComponent<RenderToTextureComponent>();

		// Create the render target
		mTarget.mClearColor = glm::vec4(resource->mClearColor.convert<RGBColorFloat>().toVec3(), 1.0f);

		// Bind textures to target
		mTarget.mColorTexture = resource->mOutputTexture;

		// Initialize target
		if (!mTarget.init(errorState))
			return false;

		// Now create a plane and initialize it.
		// The model matrix is computed on draw and used to scale the model to fit target bounds.
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		mPlane.mUsage = EMeshDataUsage::Static;
		mPlane.mCullMode = ECullMode::Back;
		mPlane.mColumns = 1;
		mPlane.mRows = 1;
		
		if (!mPlane.init(errorState))
			return false;

		// Get render service
		mService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mService != nullptr);

		// Create material instance
		if (!mMaterialInstance.init(*mService, resource->mMaterialInstanceResource, errorState))
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
			false;

		// Get view matrix (optional)
		utility::ErrorState view_error;
		mViewMatrixUniform = ensureUniform(uniform::viewMatrix, *mMVPStruct, view_error);

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		return true;
	}


	IRenderTarget& RenderToTextureComponentInstance::getTarget()
	{
		return mTarget;
	}


	nap::Texture2D& RenderToTextureComponentInstance::getOutputTexture()
	{
		return mTarget.getColorTexture();
	}


	void RenderToTextureComponentInstance::draw()
	{
		VkCommandBuffer command_buffer = mService->getCurrentCommandBuffer();

		// Create orthographic projection matrix
		glm::ivec2 size = mTarget.getBufferSize();

		// Create projection matrix
		glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)size.x, 0.0f, (float)size.y);

		// Call on draw
		mTarget.beginRendering();
		onDraw(mTarget, command_buffer, glm::mat4(), proj_matrix);
		mTarget.endRendering();
	}


	bool RenderToTextureComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	nap::MaterialInstance& RenderToTextureComponentInstance::getMaterialInstance()
	{
		return mMaterialInstance;
	}


	void RenderToTextureComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
		VkDescriptorSet descriptor_set = mMaterialInstance.update();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
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


	UniformMat4Instance* RenderToTextureComponentInstance::ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error)
	{
		// Get matrix binding
		UniformMat4Instance* found_mat = mvpStruct.getOrCreateUniform<UniformMat4Instance>(uniformName);
		if(!error.check(found_mat != nullptr, 
			"%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;
		return found_mat;
	}
}
