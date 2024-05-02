/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderfrustumcomponent.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <mathutils.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <nap/logger.h>
#include <descriptorsetcache.h>
#include <orthocameracomponent.h>
#include <perspcameracomponent.h>
#include <constantshader.h>

RTTI_BEGIN_CLASS(nap::RenderFrustumComponent)
	RTTI_PROPERTY("Line Width",	&nap::RenderFrustumComponent::mLineWidth,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",		&nap::RenderFrustumComponent::mColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Opacity",	&nap::RenderFrustumComponent::mOpacity,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendMode",	&nap::RenderFrustumComponent::mBlendMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",	&nap::RenderFrustumComponent::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderFrustumComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	/**
	 * Creates the uniform with the given name, logs an error when not available.
	 * @return the uniform, nullptr if not available.
	 */
	template<typename T>
	static T* getUniform(const std::string& uniformName, UniformStructInstance& uniformStruct, utility::ErrorState& error)
	{
		T* found_uniform = uniformStruct.getOrCreateUniform<T>(uniformName);
		return error.check(found_uniform != nullptr,
			"Unable to get or create uniform with name: %s in struct: %s", uniformName.c_str(), uniformStruct.getDeclaration().mName.c_str()) ?
			found_uniform : nullptr;
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderFrustumComponent
	//////////////////////////////////////////////////////////////////////////

	void RenderFrustumComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(CameraComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderFrustumComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderFrustumComponentInstance::RenderFrustumComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>()),
		mFrustumMesh((*entity.getCore()))
	{ }


	bool RenderFrustumComponentInstance::init(utility::ErrorState& errorState)
	{
		// Cache resource
		mResource = getComponent<RenderFrustumComponent>();

		// Ensure there is a camera component
		mCamera = getEntityInstance()->findComponent<CameraComponentInstance>();
		if (!errorState.check(mCamera != nullptr, "%s: missing camera component", mID.c_str()))
			return false;

		// Ensure there is a camera transform component
		mCameraTransform = mCamera->getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mCameraTransform != nullptr, "%s: missing camera transform component", mID.c_str()))
			return false;

		// Initialize base class
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get Gnomon material
		Material* constant_material = mRenderService->getOrCreateMaterial<ConstantShader>(errorState);
		if (!errorState.check(constant_material != nullptr, "%s: unable to get or create constant material", mID.c_str()))
			return false;

		// Create resource for the constant material instance
		mMaterialInstanceResource.mBlendMode = mResource->mBlendMode;
		mMaterialInstanceResource.mDepthMode = mResource->mDepthMode;
		mMaterialInstanceResource.mMaterial = constant_material;

		// Create constant material instance
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in shader: %s",
			this->mID.c_str(), uniform::mvpStruct, RTTI_OF(ConstantShader).get_name().data()))
			return false;

		// Get all matrices
		mModelMatUniform = getUniform<UniformMat4Instance>(uniform::modelMatrix, *mMVPStruct, errorState);
		mProjectMatUniform = getUniform<UniformMat4Instance>(uniform::projectionMatrix, *mMVPStruct, errorState);
		mViewMatUniform = getUniform<UniformMat4Instance>(uniform::viewMatrix, *mMVPStruct, errorState);
		if (mModelMatUniform == nullptr || mProjectMatUniform == nullptr || mViewMatUniform == nullptr)
			return false;

		// Get all constant uniforms
		mUBOStruct = mMaterialInstance.getOrCreateUniform(uniform::constant::uboStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform struct: %s in shader: %s",
			this->mID.c_str(), uniform::constant::uboStruct, RTTI_OF(ConstantShader).get_name().data()))
			return false;

		mColorUniform = getUniform<UniformVec3Instance>(uniform::constant::color, *mUBOStruct, errorState);
		mAlphaUniform = getUniform<UniformFloatInstance>(uniform::constant::alpha, *mUBOStruct, errorState);
		if (mColorUniform == nullptr || mAlphaUniform == nullptr)
			return false;

		// Set color & opacity
		mColorUniform->setValue(mResource->mColor.toVec3());
		mAlphaUniform->setValue(mResource->mOpacity);

		// Initialize frustum mesh
		mFrustumMesh.mUsage = EMemoryUsage::DynamicWrite;
		if (!errorState.check(mFrustumMesh.init(errorState), "Unable to create frustrum mesh"))
			return false;

		// Create mesh / material combo that can be rendered to target
		mRenderableMesh = mRenderService->createRenderableMesh(mFrustumMesh, mMaterialInstance, errorState);
		return mRenderableMesh.isValid();
	}


	bool RenderFrustumComponentInstance::updateFrustum(utility::ErrorState& errorState)
	{
		auto& positions = mFrustumMesh.getMeshInstance().getOrCreateAttribute<glm::vec3>(vertexid::position).getData();
		assert(mFrustumMesh.getNormalizedLineBox().size() == positions.size());

		if (mCamera->get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance)))
		{
			auto inv_proj_matrix = glm::inverse(mCamera->getProjectionMatrix());
			for (uint i = 0; i < mFrustumMesh.getNormalizedLineBox().size(); i++)
			{
				auto view_edge = inv_proj_matrix * glm::vec4(mFrustumMesh.getNormalizedLineBox()[i], 1.0f);
				positions[i] = view_edge;
			}
		}
		else if (mCamera->get_type().is_derived_from(RTTI_OF(PerspCameraComponentInstance)))
		{
			auto inv_proj_matrix = glm::inverse(mCamera->getProjectionMatrix());
			for (uint i = 0; i < mFrustumMesh.getNormalizedLineBox().size(); i++)
			{
				auto view_edge = inv_proj_matrix * glm::vec4(mFrustumMesh.getNormalizedLineBox()[i], 1.0f);
				positions[i] = view_edge / view_edge.w; // Perspective divide
			}
		}
		else
		{
			errorState.fail("Unsupported camera type");
			return false;
		}

		utility::ErrorState error_state;
		if (!mFrustumMesh.getMeshInstance().update(error_state))
			assert(false);

		return true;
	}


	void RenderFrustumComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Update frustum
		{
			utility::ErrorState error_state;
			if (!updateFrustum(error_state))
				nap::Logger::warn(error_state.toString());
		}

		// Set mvp matrices
		assert(mProjectMatUniform != nullptr);
		mProjectMatUniform->setValue(projectionMatrix);
		assert(mViewMatUniform != nullptr);
		mViewMatUniform->setValue(viewMatrix);
		assert(mModelMatUniform != nullptr);
		mModelMatUniform->setValue(mCameraTransform->getGlobalTransform());

		// Acquire new / unique descriptor set before rendering
		const DescriptorSet& descriptor_set = mMaterialInstance.update();

		// Fetch and bind pipeline
		assert(mRenderableMesh.isValid());
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mResource->mLineWidth);

		// Draw meshes
		GPUMesh& gpu_mesh = mFrustumMesh.getMeshInstance().getGPUMesh();
		const IndexBuffer& index_buffer = gpu_mesh.getIndexBuffer(0);
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);
	}
}
