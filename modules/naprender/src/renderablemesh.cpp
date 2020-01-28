#include "renderablemesh.h"
#include "renderablemeshcomponent.h"
#include "nvertexbuffer.h"
#include "material.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, VkPipelineLayout layout, VkPipeline pipeline) :
		mMaterialInstance(&materialInstance),
		mMesh(&mesh),
		mPipelineLayout(layout),
		mPipeline(pipeline)
	{
		mMaterialInstance->pipelineStateChanged.connect(mPipelineStateChangedSlot);

		opengl::GPUMesh& gpu_mesh = mesh.getMeshInstance().getGPUMesh();
		Material& material = *materialInstance.getMaterial();
		for (auto& kvp : material.getShader()->getShader().getAttributes())
		{
			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			assert(material_binding != nullptr);

			opengl::VertexAttributeBuffer& vertex_buffer = gpu_mesh.getVertexAttributeBuffer(material_binding->mMeshAttributeID);
			mVertexBuffers.push_back(vertex_buffer.getBuffer());
			mVertexBufferOffsets.push_back(0);
		}
	}

	RenderableMesh::RenderableMesh(const RenderableMesh& renderableMesh) :
		mMaterialInstance(renderableMesh.mMaterialInstance),
		mMesh(renderableMesh.mMesh),
		mPipelineLayout(renderableMesh.mPipelineLayout),
		mPipeline(renderableMesh.mPipeline),
		mVertexBuffers(renderableMesh.mVertexBuffers),
		mVertexBufferOffsets(renderableMesh.mVertexBufferOffsets)
	{
		mMaterialInstance->pipelineStateChanged.connect(mPipelineStateChangedSlot);
	}

	RenderableMesh& RenderableMesh::operator=(const RenderableMesh& renderableMesh)
	{
		if (this != &renderableMesh)
		{
			mMaterialInstance = renderableMesh.mMaterialInstance;
			mMesh = renderableMesh.mMesh;
			mPipelineLayout = renderableMesh.mPipelineLayout;
			mPipeline = renderableMesh.mPipeline;
			mVertexBuffers = renderableMesh.mVertexBuffers;
			mVertexBufferOffsets = renderableMesh.mVertexBufferOffsets;

			mMaterialInstance->pipelineStateChanged.connect(mPipelineStateChangedSlot);
		}
		return *this;
	}

	void RenderableMesh::onPipelineStateChanged(const MaterialInstance& materialInstance, RenderService& renderService)
	{
		renderService.recreatePipeline(*this, mPipelineLayout, mPipeline);;
	}
}

