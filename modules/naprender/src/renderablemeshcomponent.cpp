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
#include "nindexbuffer.h"

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
		RenderableComponentInstance(entity, resource)
	{
	}


	bool RenderableMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		RenderableMeshComponent* resource = getComponent<RenderableMeshComponent>();

		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// A mesh isn't required, it may be set by a derived class or by some other code through setMesh
		// If it is set, we create a renderablemesh from it
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
	void RenderableMeshComponentInstance::onDraw(opengl::RenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Get global transform
		const glm::mat4x4& model_matrix = mTransformComponent->getGlobalTransform();

		// Bind material
		MaterialInstance& mat_instance = getMaterialInstance();

		// Set projection uniform in shader
		UniformMat4Instance& projectionUniform = mat_instance.getOrCreateUniform("nap").getOrCreateUniform<UniformMat4Instance>(projectionMatrixUniform);
		projectionUniform.setValue(projectionMatrix);

		// Set view uniform in shader
		UniformMat4Instance& viewUniform = mat_instance.getOrCreateUniform("nap").getOrCreateUniform<UniformMat4Instance>(viewMatrixUniform);
		viewUniform.setValue(viewMatrix);

		// Set model matrix uniform in shader
		UniformMat4Instance& modelUniform = mat_instance.getOrCreateUniform("nap").getOrCreateUniform<UniformMat4Instance>(modelMatrixUniform);
		modelUniform.setValue(model_matrix);

		VkDescriptorSet descriptor_set = mat_instance.update();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderableMesh.getPipeline());

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = renderTarget.getSize().x;
		viewport.height = renderTarget.getSize().y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// Gather draw info
		MeshInstance& mesh_instance = getMeshInstance();
		opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();

		Material& material = mRenderableMesh.getMaterialInstance().getMaterial();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderableMesh.getPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);

		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		if (mClipRect.hasWidth() && mClipRect.hasHeight())
		{
			VkRect2D rect;
			rect.offset.x = mClipRect.getMin().x;
			rect.offset.y = mClipRect.getMin().y;
			rect.extent.width = mClipRect.getWidth();
			rect.extent.height = mClipRect.getHeight();
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}

		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const opengl::IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}


	MaterialInstance& RenderableMeshComponentInstance::getMaterialInstance()
	{
		return mRenderableMesh.getMaterialInstance();
	}
} 
