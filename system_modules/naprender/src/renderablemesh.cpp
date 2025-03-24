/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderablemesh.h"
#include "renderablemeshcomponent.h"
#include "gpubuffer.h"
#include "material.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance) :
		mMaterialInstance(&materialInstance), mMesh(&mesh)
	{
		GPUMesh& gpu_mesh = mMesh->getMeshInstance().getGPUMesh();
		const Material& material = mMaterialInstance->getMaterial();
		for (auto& kvp : material.getShader().getAttributes())
		{
			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			assert(material_binding != nullptr);

			GPUBuffer& vertex_buffer = gpu_mesh.getVertexBuffer(material_binding->mMeshAttributeID);
			vertex_buffer.bufferChanged.connect(mVertexBufferDataChangedSlot);
			mVertexBufferOffsets.push_back(0);
		}
	}


	void RenderableMesh::move(RenderableMesh&& other)
	{
		mMaterialInstance = other.mMaterialInstance;
		other.mMaterialInstance = nullptr;
		mMesh = other.mMesh;
		other.mMesh = nullptr;
		mVertexBuffers = std::move(other.mVertexBuffers);
		mVertexBufferOffsets = std::move(other.mVertexBufferOffsets);
		mVertexBuffersDirty = other.mVertexBuffersDirty;

        // Don't copy the slot -> that will also copy the function that is called
        // Only copy potential triggers (signals)
        mVertexBufferDataChangedSlot.copyCauses(other.mVertexBufferDataChangedSlot);
	}


	void RenderableMesh::copy(const RenderableMesh& rhs)
	{
		mMaterialInstance = rhs.mMaterialInstance;
		mMesh = rhs.mMesh;
		mVertexBuffers = rhs.mVertexBuffers;
		mVertexBufferOffsets = rhs.mVertexBufferOffsets;
		mVertexBuffersDirty = rhs.mVertexBuffersDirty;

        // Don't copy the slot -> that will also copy the function that is called
        // Only copy potential triggers (signals)
        mVertexBufferDataChangedSlot.copyCauses(rhs.mVertexBufferDataChangedSlot);
	}


	const std::vector<VkBuffer>& RenderableMesh::getVertexBuffers()
	{
		if (!mVertexBuffersDirty)
			return mVertexBuffers;

		mVertexBuffers.clear();

		GPUMesh& gpu_mesh = mMesh->getMeshInstance().getGPUMesh();
		const Material& material = mMaterialInstance->getMaterial();
		for (auto& kvp : material.getShader().getAttributes())
		{
			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			assert(material_binding != nullptr);

			GPUBufferNumeric& vertex_buffer = gpu_mesh.getVertexBuffer(material_binding->mMeshAttributeID);
			mVertexBuffers.push_back(vertex_buffer.getBuffer());
		}

		mVertexBuffersDirty = false;
		return mVertexBuffers;
	}


	int RenderableMesh::getVertexBufferBindingIndex(const std::string& meshVertexAttributeID) const
	{
		GPUMesh& gpu_mesh = mMesh->getMeshInstance().getGPUMesh();
		const Material& material = mMaterialInstance->getMaterial();

		int binding = 0;
		for (auto& kvp : material.getShader().getAttributes())
		{
			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			assert(material_binding != nullptr);

			// Override the position mesh attribute
			if (material_binding->mMeshAttributeID == meshVertexAttributeID)
				return binding;

			++binding;
		}
		assert(false);
		return -1;
	}

}

