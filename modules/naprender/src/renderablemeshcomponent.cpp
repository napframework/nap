// Local Includes
#include "renderablemeshcomponent.h"
#include "mesh.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"
#include "indexbuffer.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::RenderableMeshComponent)
	RTTI_PROPERTY("Mesh",				&nap::RenderableMeshComponent::mMesh,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableMeshComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LineWidth",			&nap::RenderableMeshComponent::mLineWidth,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClipRect",			&nap::RenderableMeshComponent::mClipRect,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	//RTTI_FUNCTION("getMaterialInstance", &nap::RenderableMeshComponentInstance::getMaterialInstance)
RTTI_END_CLASS

namespace nap
{
	void RenderableMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}


	RenderableMeshComponentInstance::RenderableMeshComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<nap::RenderService>())
	{ }


	bool RenderableMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Initialize material based on resource
		RenderableMeshComponent* resource = getComponent<RenderableMeshComponent>();
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// A mesh isn't required, it may be set by a derived class or by some other code through setMesh.
		// If it is set we create a renderable mesh
		if (resource->mMesh != nullptr)
		{
			mRenderableMesh = createRenderableMesh(*resource->mMesh, mMaterialInstance, errorState);
			if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
				return false;
		}

		// Ensure there is a transform component
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
 		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
 			return false;

		// Copy cliprect. Any modifications are done per instance
		mClipRect = resource->mClipRect;

		// Copy line width, ensure it's supported
		mLineWidth = resource->mLineWidth;
		if (mLineWidth > 1.0f && !mRenderService->getWideLinesSupported())
		{
			nap::Logger::warn("Unsupported line width: %.02f", mLineWidth);
			mLineWidth = 1.0f;
		}

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


	RenderableMesh RenderableMeshComponentInstance::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		return render_service->createRenderableMesh(mesh, materialInstance, errorState);
	}


	RenderableMesh RenderableMeshComponentInstance::createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState)
	{
		return createRenderableMesh(mesh, mMaterialInstance, errorState);
	}


	void RenderableMeshComponentInstance::setMesh(const RenderableMesh& mesh)
	{
		assert(mesh.isValid());
		mRenderableMesh = mesh;
	}


	// Draw Mesh
	void RenderableMeshComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if(mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);
		
		if(mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if(mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		VkDescriptorSet descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

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


	MaterialInstance& RenderableMeshComponentInstance::getMaterialInstance()
	{
		return mRenderableMesh.getMaterialInstance();
	}
} 
