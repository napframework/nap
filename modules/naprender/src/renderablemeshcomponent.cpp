// Local Includes
#include "renderablemeshcomponent.h"
#include "mesh.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>

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

		if (!mMaterialInstance.init(getEntityInstance()->getCore()->getService<RenderService>()->getRenderer(), resource->mMaterialInstanceResource, errorState))
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
	void RenderableMeshComponentInstance::onDraw(VkCommandBuffer commandBuffer, int frameIndex, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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
		Material* comp_mat = mat_instance.getMaterial();
		UniformMat4* projectionUniform = comp_mat->findUniform<UniformMat4>(projectionMatrixUniform);
		if (projectionUniform != nullptr)
			projectionUniform->setValue(projectionMatrix);

		// Set view uniform in shader
		UniformMat4* viewUniform = comp_mat->findUniform<UniformMat4>(viewMatrixUniform);
		if (viewUniform != nullptr)
			viewUniform->setValue(viewMatrix);

		// Set model matrix uniform in shader
		UniformMat4* modelUniform = comp_mat->findUniform<UniformMat4>(modelMatrixUniform);
		if (modelUniform != nullptr)
			modelUniform->setValue(model_matrix);

		mat_instance.pushUniforms(frameIndex);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderableMesh.getPipeline());

		// Gather draw info
		MeshInstance& mesh_instance = getMeshInstance();
		opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();

		Material& material = *mRenderableMesh.getMaterialInstance().getMaterial();

		VkDescriptorSet descriptor_set = mRenderableMesh.getMaterialInstance().getDescriptorSet(frameIndex);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderableMesh.getPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);

		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceSize> vertexBufferOffsets;

		for (auto& kvp : material.getShader()->getShader().getAttributes())
		{
			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			assert(material_binding != nullptr);

			opengl::VertexAttributeBuffer& vertex_buffer = mesh.getVertexAttributeBuffer(material_binding->mMeshAttributeID);
			vertexBuffers.push_back(vertex_buffer.getBuffer());
			vertexBufferOffsets.push_back(0);
		}

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());



		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const opengl::IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

		/*

		// Get global transform
		const glm::mat4x4& model_matrix = mTransformComponent->getGlobalTransform();

		// Bind material
		MaterialInstance& mat_instance = getMaterialInstance();
		mat_instance.bind();

		// Set projection uniform in shader
		Material* comp_mat = mat_instance.getMaterial();
		UniformMat4* projectionUniform = comp_mat->findUniform<UniformMat4>(projectionMatrixUniform);
		if (projectionUniform != nullptr)
			projectionUniform->setValue(projectionMatrix);

		// Set view uniform in shader
		UniformMat4* viewUniform = comp_mat->findUniform<UniformMat4>(viewMatrixUniform);
		if (viewUniform != nullptr)
			viewUniform->setValue(viewMatrix);

		// Set model matrix uniform in shader
		UniformMat4* modelUniform = comp_mat->findUniform<UniformMat4>(modelMatrixUniform);
		if (modelUniform != nullptr)
			modelUniform->setValue(model_matrix);

		// Prepare blending
		mat_instance.pushBlendMode();

		// Push all shader uniforms
		mat_instance.pushUniforms();

		// Bind mesh for rendering
		mRenderableMesh.bind();

		// If a cliprect was set, enable scissor and set correct values
		if (mClipRect.hasWidth() && mClipRect.hasHeight())
		{
			opengl::enableScissorTest(false);
			glScissor(mClipRect.getMin().x, mClipRect.getMin().y, mClipRect.getWidth(), mClipRect.getHeight());
		}

		// Gather draw info
		MeshInstance& mesh_instance = getMeshInstance();
		const opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Draw all shapes associated with the mesh
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			MeshShape& shape = mesh_instance.getShape(index);
			const opengl::IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			
			GLenum draw_mode = getGLMode(shape.getDrawMode());
			GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());

// 			index_buffer.bind();
// 			glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
// 			index_buffer.unbind();
		}

		mat_instance.unbind();
		mRenderableMesh.unbind();
		opengl::enableScissorTest(false);*/
	}


	MaterialInstance& RenderableMeshComponentInstance::getMaterialInstance()
	{
		return mRenderableMesh.getMaterialInstance();
	}
} 
