/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderquiltcameraboxcomponent.h"
#include "quiltrendertarget.h"

// External Includes
#include <mesh.h>
#include <renderglobals.h>
#include <material.h>
#include <renderservice.h>
//#include <indexbuffer.h>
#include <renderglobals.h>
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <transformcomponent.h>
#include <rtti/typeinfo.h>
#include <glm/gtc/matrix_transform.hpp>

RTTI_BEGIN_CLASS(nap::RenderQuiltCameraBoxComponent)
	RTTI_PROPERTY("QuiltCameraComponent",	&nap::RenderQuiltCameraBoxComponent::mQuiltCameraComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",		&nap::RenderQuiltCameraBoxComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PolygonMode",			&nap::RenderQuiltCameraBoxComponent::mPolygonMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LineWidth",				&nap::RenderQuiltCameraBoxComponent::mLineWidth,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BoxDepth",				&nap::RenderQuiltCameraBoxComponent::mBoxDepth,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderQuiltCameraBoxComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	void RenderQuiltCameraBoxComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}

		
	RenderQuiltCameraBoxComponentInstance::RenderQuiltCameraBoxComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<nap::RenderService>())
	{ }


	bool RenderQuiltCameraBoxComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Initialize material based on resource
		RenderQuiltCameraBoxComponent* resource = getComponent<RenderQuiltCameraBoxComponent>();
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// Create a box mesh
		mBoxMesh = std::make_unique<QuiltCameraBoxMesh>(*getEntityInstance()->getCore());
		mBoxMesh->mCullMode = ECullMode::Front;
		mBoxMesh->mPolygonMode = resource->mPolygonMode;
		if (!mBoxMesh->init(errorState))
			return false;

		mRenderableMesh = createRenderableMesh(*mBoxMesh, mMaterialInstance, errorState);
		if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
			return false;

		// Ensure there is a transform component
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
 		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
 			return false;

		// Copy line width, ensure it's supported
		mLineWidth = resource->mLineWidth;
		if (mLineWidth > 1.0f && !mRenderService->getWideLinesSupported())
		{
			nap::Logger::warn("Unsupported line width: %.02f", mLineWidth);
			mLineWidth = 1.0f;
		}

		mBoxDepth = resource->mBoxDepth;

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		UniformStructInstance* mvp_struct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (mvp_struct != nullptr)
		{
			mModelMatUniform = mvp_struct->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
			mViewMatUniform = mvp_struct->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
			mProjectMatUniform = mvp_struct->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
		}
		return true;
	}


	void RenderQuiltCameraBoxComponentInstance::update(double deltaTime)
	{
		// Fetch the quilt camera transform
		auto& camera_transform = (*mQuiltCameraComponent).getEntityInstance()->getComponent<TransformComponentInstance>();

		// Compute the distance between the camera and the back plane of the box
		float backplane_distance = (*mQuiltCameraComponent).getDistanceToFocalPlane() + mBoxDepth;

		// Ensure the local transform of this component represents the location of the back plane.
		// This ensures the depth sorter considers this object to be behind other components inside the box.
		mTransformComponent->setLocalTransform(camera_transform.getGlobalTransform() * glm::translate({}, glm::vec3(0.0f, 0.0f, backplane_distance)));
	}


	RenderableMesh RenderQuiltCameraBoxComponentInstance::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		return render_service->createRenderableMesh(mesh, materialInstance, errorState);
	}


	RenderableMesh RenderQuiltCameraBoxComponentInstance::createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState)
	{
		return createRenderableMesh(mesh, mMaterialInstance, errorState);
	}


	void RenderQuiltCameraBoxComponentInstance::setMesh(const RenderableMesh& mesh)
	{
		assert(mesh.isValid());
		mRenderableMesh = mesh;
	}


	// Draw Mesh
	void RenderQuiltCameraBoxComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Fetch the quilt camera transform
		auto& camera_transform = (*mQuiltCameraComponent).getEntityInstance()->getComponent<TransformComponentInstance>().getGlobalTransform();

		// The focal matrix brings an object in camera view space in focus by applying a translation in the lookat direction of the camera
		glm::mat4 focal_matrix = glm::translate({}, glm::vec3(0.0f, 0.0f, -(*mQuiltCameraComponent).getDistanceToFocalPlane() - mBoxDepth * 0.5f));

		// Scale the box
		float camera_size = (*mQuiltCameraComponent).getCameraSize();
		float aspect = renderTarget.getBufferSize().x / static_cast<float>(renderTarget.getBufferSize().y);
		const glm::mat4 scale_matrix = glm::scale({}, glm::vec3(camera_size * 2.0f * aspect, camera_size * 2.0f, mBoxDepth));

		// Set mvp matrices if present in material
		if(mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		// Get the camera view matrix without a horizontal offset (as is returned by nap::QuiltCameraComponentInstance::getViewMatrix())
		if(mViewMatUniform != nullptr)
			mViewMatUniform->setValue(focal_matrix * viewMatrix * camera_transform);

		// The model matrix simply scales the box to the camera dimensions
		if(mModelMatUniform != nullptr)
			mModelMatUniform->setValue(scale_matrix);

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		auto& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw meshes
		MeshInstance& mesh_instance = getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);
	}


	MaterialInstance& RenderQuiltCameraBoxComponentInstance::getMaterialInstance()
	{
		return mRenderableMesh.getMaterialInstance();
	}


	bool RenderQuiltCameraBoxComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(QuiltCameraComponentInstance));
	}
} 
