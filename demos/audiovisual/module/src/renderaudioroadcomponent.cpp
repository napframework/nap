#include "renderaudioroadcomponent.h"

// External Includes
#include <entity.h>
#include <computecomponent.h>
#include <renderglobals.h>
#include <planemeshvec4.h>
#include <mesh.h>

// nap::RenderAudioRoadComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderAudioRoadComponent)
	RTTI_PROPERTY("AudioRoadComponent",			&nap::RenderAudioRoadComponent::mAudioRoadComponent,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::RenderAudioRoadComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderAudioRoadComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool RenderAudioRoadComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<RenderAudioRoadComponent>();

		// Force vec4 plane mesh
		if (!errorState.check(mResource->mMesh.get()->get_type() == RTTI_OF(PlaneMeshVec4), "Mesh must be of type `nap::PlaneMeshVec4`"))
			return false;

		if (!RenderableMeshComponentInstance::init(errorState))
			return false;

		return true;
	}


	void RenderAudioRoadComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
		{
			// Copy the ordered vector of VkBuffers from the renderable mesh
			std::vector<VkBuffer> vertex_buffers = mRenderableMesh.getVertexBuffers();

			// Override position vertex attribute buffer with storage buffer.
			// We do this by first fetching the internal buffer binding index of the position vertex attribute.
			int position_attr_binding_idx = mRenderableMesh.getVertexBufferBindingIndex(vertexid::position);
			if (position_attr_binding_idx >= 0)
			{
				// Overwrite the VkBuffer under the previously fetched position vertex attribute index.
				vertex_buffers[position_attr_binding_idx] = mAudioRoadComponent->getPositionBuffer().getBuffer();
			}

			// Repeat for the normal attribute
			int normal_attr_binding_idx = mRenderableMesh.getVertexBufferBindingIndex(vertexid::normal);
			if (normal_attr_binding_idx >= 0)
				vertex_buffers[normal_attr_binding_idx] = mAudioRoadComponent->getNormalBuffer().getBuffer();

			// Get offsets
			const std::vector<VkDeviceSize>& offsets = mRenderableMesh.getVertexBufferOffsets();

			// Bind buffers
			// The shader will now use the storage buffer updated by the compute shader as a vertex buffer when rendering the current mesh.
			vkCmdBindVertexBuffers(commandBuffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
		}

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
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

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
}
