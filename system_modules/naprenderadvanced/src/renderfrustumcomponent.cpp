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

RTTI_BEGIN_CLASS(nap::RenderFrustumComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderFrustumComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderFrustumComponent
	//////////////////////////////////////////////////////////////////////////

	void RenderFrustumComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(CameraComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderFrustumComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderFrustumComponentInstance::RenderFrustumComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>()),
		mFrustumMesh(std::make_unique<BoxFrameMesh>(*entity.getCore()))
	{ }


	bool RenderFrustumComponentInstance::init(utility::ErrorState& errorState)
	{
		// Cache resource
		mResource = getComponent<RenderFrustumComponent>();

		// Ensure there is a camera component
		mCamera = getEntityInstance()->findComponent<CameraComponentInstance>();
		if (!errorState.check(mCamera != nullptr, "%s: missing camera component", mID.c_str()))
			return false;

		// Initialize base class
		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Initialize frustum mesh
		mFrustumMesh->mPolygonMode = EPolygonMode::Line;
		mFrustumMesh->mUsage = EMemoryUsage::DynamicWrite;
		if (!errorState.check(mFrustumMesh->init(errorState), "Unable to create particle mesh"))
			return false;

		// Create renderable mesh
		RenderableMesh renderable_mesh = createRenderableMesh(*mFrustumMesh, errorState);
		if (!renderable_mesh.isValid())
			return false;

		// Set the frustum mesh to be used when drawing
		setMesh(renderable_mesh);

		return true;
	}


	bool RenderFrustumComponentInstance::updateFrustum(utility::ErrorState& errorState)
	{
		auto& positions = getMeshInstance().getOrCreateAttribute<glm::vec3>(vertexid::position).getData();
		assert(mFrustumMesh->getNormalizedLineBox().size() == positions.size());

		if (mCamera->get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance)))
		{
			auto inv_proj_matrix = glm::inverse(mCamera->getProjectionMatrix());
			for (uint i = 0; i < mFrustumMesh->getNormalizedLineBox().size(); i++)
			{
				auto view_edge = inv_proj_matrix * glm::vec4(mFrustumMesh->getNormalizedLineBox()[i], 1.0f);
				positions[i] = view_edge;
			}
		}
		else if (mCamera->get_type().is_derived_from(RTTI_OF(PerspCameraComponentInstance)))
		{
			auto inv_proj_matrix = glm::inverse(mCamera->getProjectionMatrix());
			for (uint i = 0; i < mFrustumMesh->getNormalizedLineBox().size(); i++)
			{
				auto view_edge = inv_proj_matrix * glm::vec4(mFrustumMesh->getNormalizedLineBox()[i], 1.0f);
				positions[i] = view_edge / view_edge.w; // Perspective divide
			}
		}
		else
		{
			errorState.fail("Unsupported camera type");
			return false;
		}

		utility::ErrorState error_state;
		if (!getMeshInstance().update(error_state))
			assert(false);

		return true;
	}


	void RenderFrustumComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransformComponent->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

		// Update frustum
		{
			utility::ErrorState error_state;
			if (!updateFrustum(error_state))
				nap::Logger::warn(error_state.toString());
		}

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertex_buffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());

		// TODO: move to push/pop cliprect on RenderTarget once it has been ported
		bool has_clip_rect = mClipRect.hasWidth() && mClipRect.hasHeight();
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = mClipRect.getMin().x;
			rect.offset.y = mClipRect.getMin().y;
			rect.extent.width = mClipRect.getWidth();
			rect.extent.height = mClipRect.getHeight();
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw meshes
		MeshInstance& mesh_instance = getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		const IndexBuffer& index_buffer = mesh.getIndexBuffer(0);
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);

		// Restore clipping
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = 0;
			rect.offset.y = 0;
			rect.extent.width = renderTarget.getBufferSize().x;
			rect.extent.height = renderTarget.getBufferSize().y;
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}
	}


	RenderFrustumComponent& RenderFrustumComponentInstance::getResource()
	{
		return *mResource;
	}
}
