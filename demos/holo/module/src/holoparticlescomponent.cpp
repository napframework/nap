/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "HoloParticlesComponent.h"

// External Includes
#include <entity.h>
#include <rect.h>
#include <glm/gtx/transform.hpp>
#include <mathutils.h>
#include <nap/core.h>
#include <renderservice.h>
#include <renderglobals.h>
#include <nap/logger.h>
#include <descriptorsetcache.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::HoloParticlesComponent)
	RTTI_PROPERTY("Mesh",						&nap::HoloParticlesComponent::mMesh,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",			&nap::HoloParticlesComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NumParticles",				&nap::HoloParticlesComponent::mNumParticles,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SpeedParameter",				&nap::HoloParticlesComponent::mSpeedParameter,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SizeParameter",				&nap::HoloParticlesComponent::mSizeParameter,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RainbowBlendParameter",		&nap::HoloParticlesComponent::mRainbowBlendParameter,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraTransformComponent",	&nap::HoloParticlesComponent::mCameraTransformComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HoloParticlesComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Constants
	//////////////////////////////////////////////////////////////////////////

	namespace uniform
	{
		constexpr const char* VERTUBO = "VERTUBO";
		constexpr const char* FRAGUBO = "FRAGUBO";
	}

	//////////////////////////////////////////////////////////////////////////
	// HoloParticlesComponentInstance
	//////////////////////////////////////////////////////////////////////////

	HoloParticlesComponentInstance::HoloParticlesComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>())
	{ }


	bool HoloParticlesComponentInstance::init(utility::ErrorState& errorState)
	{
		// Initialize base class
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		HoloParticlesComponent* resource = getComponent<HoloParticlesComponent>();
		mCount = resource->mNumParticles;

		// Initialize our material instance based on values in the resource
		if (!mMaterialInstance.init(*mRenderService, resource->mMaterialInstanceResource, errorState))
			return false;

		// Get handle to matrices, which we set in the draw call
		mProjectionUniform = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct)->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
		mViewUniform = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct)->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
		mModelUniform = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct)->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);

		// Bind the particle mesh to the material and create a VAO
		RenderableMesh render_mesh = mRenderService->createRenderableMesh(*resource->mMesh, mMaterialInstance, errorState);
		if (!errorState.check(render_mesh.isValid(), "%s, mesh: %s invalid", resource->mID.c_str(), resource->mMesh->mID.c_str()))
			return false;

		// Set the particle mesh to be used when drawing
		mRenderableMesh = std::move(render_mesh);

		auto* uni_struct = mMaterialInstance.getOrCreateUniform(uniform::VERTUBO);
		auto* positions_uni = uni_struct->getOrCreateUniform<UniformVec3ArrayInstance>("initialPositions");
		if (!errorState.check(mCount <= positions_uni->getNumElements(), "Particle count exceeds positions uniform"))
			return false;

		// Set initial positions once
		for (int i = 0; i < mCount; i++)
		{
			const auto random_position = math::random<glm::vec3>({ -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
			positions_uni->setValue(random_position, i);
		}

		// Start elapsed time at a random offset
		mElapsedTime = math::random(10.0f, 10000.0f);

		mResource = resource;
		return true;
	}


	void HoloParticlesComponentInstance::update(double deltaTime)
	{
		// Parameters
		mSize = mResource->mSizeParameter != nullptr ? mResource->mSizeParameter->mValue : mSize;
		mSpeed = (mResource->mSpeedParameter != nullptr ? mResource->mSpeedParameter->mValue : mSpeed);
		mRainbowBlend = (mResource->mRainbowBlendParameter != nullptr ? mResource->mRainbowBlendParameter->mValue : mRainbowBlend);

		// Update time variables
		mDeltaTime = deltaTime * static_cast<double>(mSpeed);
		mElapsedTime += mDeltaTime;
	}


	void HoloParticlesComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if (mProjectionUniform != nullptr)
			mProjectionUniform->setValue(projectionMatrix);

		if (mViewUniform != nullptr)
			mViewUniform->setValue(viewMatrix);

		if (mModelUniform != nullptr)
			mModelUniform->setValue(getEntityInstance()->findComponent<TransformComponentInstance>()->getGlobalTransform());

		// Get camera world space position and set
		glm::vec3 global_pos = math::extractPosition(mCameraTransformComponent->getGlobalTransform());

		nap::UniformStructInstance* ubo = mMaterialInstance.getOrCreateUniform(uniform::VERTUBO);
		if (ubo != nullptr)
		{
			ubo->getOrCreateUniform<nap::UniformVec3Instance>("cameraLocation")->setValue(global_pos);
			ubo->getOrCreateUniform<UniformFloatInstance>("elapsedTime")->setValue(static_cast<float>(mElapsedTime));
			ubo->getOrCreateUniform<UniformFloatInstance>("particleSize")->setValue(mSize * mSizeLimitation);
		}

		ubo = mMaterialInstance.getOrCreateUniform(uniform::FRAGUBO);
		if (ubo != nullptr)
		{
			ubo->getOrCreateUniform<nap::UniformVec3Instance>("cameraLocation")->setValue(global_pos);
			ubo->getOrCreateUniform<nap::UniformVec3Instance>("ambientColor")->setValue(renderTarget.getClearColor());
			ubo->getOrCreateUniform<nap::UniformFloatInstance>("rainbowBlend")->setValue(mRainbowBlend);
		}

		// Acquire new / unique descriptor set before rendering
		VkDescriptorSet descriptor_set = mMaterialInstance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());

		// Draw meshes
		MeshInstance& mesh_instance = mResource->mMesh->getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		const IndexBuffer& index_buffer = mesh.getIndexBuffer(0);
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		// Make use of instanced rendering by setting the `instanceCount` of `vkCmdDrawIndexed()` equal to the boid count.
		// This renders the boid mesh `mNumboids` times in a single draw call.
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), mCount, 0, 0, 0);
	}
}
