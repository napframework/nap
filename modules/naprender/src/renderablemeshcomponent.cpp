// Local Includes
#include "renderablemeshcomponent.h"
#include "mesh.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"
#include "uniforminstances.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include "indexbuffer.h"

RTTI_BEGIN_CLASS(nap::RenderableMeshComponent)
	RTTI_PROPERTY("Mesh",				&nap::RenderableMeshComponent::mMesh,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableMeshComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
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
	{
	}


	bool RenderableMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Initialize material
		RenderableMeshComponent* resource = getComponent<RenderableMeshComponent>();
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// A mesh isn't required, it may be set by a derived class or by some other code through setMesh
		// If it is set, we create a renderable-mesh from it
		if (resource->mMesh != nullptr)
		{
			mRenderableMesh = createRenderableMesh(*resource->mMesh, mMaterialInstance, errorState);
			if (!errorState.check(mRenderableMesh.isValid(), "%s: unable to create renderable mesh", mID.c_str()))
				return false;
		}

		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
 		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", mID.c_str()))
 			return false;

		// Copy cliprect. Any modifications are done per instance
		mClipRect = resource->mClipRect;

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

		// Fetch mvp uniform and update individual matrices
		MaterialInstance& mat_instance = getMaterialInstance();
		UniformStructInstance* mvp_uniform = mat_instance.getOrCreateUniform(mvpStructUniform);
		if (mvp_uniform != nullptr)
		{
			// Set projection uniform in shader
			UniformMat4Instance* projection_uniform = mvp_uniform->getOrCreateUniform<UniformMat4Instance>(projectionMatrixUniform);
			if (projection_uniform != nullptr)
				projection_uniform->setValue(projectionMatrix);

			// Set view uniform in shader
			UniformMat4Instance* view_uniform = mvp_uniform->getOrCreateUniform<UniformMat4Instance>(viewMatrixUniform);
			if (view_uniform != nullptr)
				view_uniform->setValue(viewMatrix);

			// Set model matrix uniform in shader
			UniformMat4Instance* model_uniform = mvp_uniform->getOrCreateUniform<UniformMat4Instance>(modelMatrixUniform);
			if (model_uniform != nullptr)
			{
				const glm::mat4x4& model_matrix = mTransformComponent->getGlobalTransform();
				model_uniform->setValue(model_matrix);
			}
		}

		// Acquire new / unique descriptor set before rendering
		VkDescriptorSet descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		Material& material = mat_instance.getMaterial();
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

		// Draw meshes
		MeshInstance& mesh_instance = getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

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
