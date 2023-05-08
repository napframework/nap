/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderskyboxcomponent.h"
#include "skyboxshader.h"

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

RTTI_BEGIN_CLASS(nap::RenderSkyBoxComponent)
	RTTI_PROPERTY("CubeTexture", &nap::RenderSkyBoxComponent::mCubeTexture, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderSkyBoxComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// RenderSkyBoxComponent
	//////////////////////////////////////////////////////////////////////////

	void RenderSkyBoxComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(CameraComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// RenderSkyBoxComponentInstance
	//////////////////////////////////////////////////////////////////////////

	RenderSkyBoxComponentInstance::RenderSkyBoxComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableMeshComponentInstance(entity, resource),
		mRenderService(*entity.getCore()->getService<RenderService>())
	{ }


	bool RenderSkyBoxComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<RenderSkyBoxComponent>();
		if (!errorState.check(mResource->mMesh != nullptr, utility::stringFormat("%s: Property 'Mesh' cannot be NULL", mID.c_str()).c_str()))
			return false;

		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		// Set equirectangular texture to convert
		auto* sampler = mMaterialInstance.getOrCreateSampler<SamplerCubeInstance>("cubeTexture");
		if (!errorState.check(sampler != nullptr, utility::stringFormat("%s: Incompatible shader interface", mID.c_str()).c_str()))
			return false;

		sampler->setTexture(*mResource->mCubeTexture);

		return true;
	}


	void RenderSkyBoxComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService.getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
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


	RenderSkyBoxComponent& RenderSkyBoxComponentInstance::getResource()
	{
		return *mResource;
	}
}
