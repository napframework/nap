#include "renderablemesh.h"
#include "renderablemeshcomponent.h"
#include "vertexbuffer.h"
#include "material.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance) :
		mMaterialInstance(&materialInstance),
		mMesh(&mesh)
	{
		GPUMesh& gpu_mesh = mesh.getMeshInstance().getGPUMesh();
		const Material& material = materialInstance.getMaterial();
		for (auto& kvp : material.getShader().getAttributes())
		{
			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			assert(material_binding != nullptr);

			VertexAttributeBuffer& vertex_buffer = gpu_mesh.getVertexAttributeBuffer(material_binding->mMeshAttributeID);
			mVertexBuffers.push_back(vertex_buffer.getBuffer());
			mVertexBufferOffsets.push_back(0);
		}
	}
}

