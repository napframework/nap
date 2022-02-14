/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gpumesh.h"
#include "renderservice.h"

// External includes
#include <nap/assert.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// GPUMesh
	//////////////////////////////////////////////////////////////////////////

	GPUMesh::GPUMesh(RenderService& renderService, EMemoryUsage inUsage) :
		mRenderService(&renderService),
		mUsage(inUsage)
	{ }


	const ValueGPUBuffer* GPUMesh::findVertexBuffer(const std::string& id) const
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		if (attribute != mAttributes.end())
			return attribute->second.get();
		return nullptr;
	}


	ValueGPUBuffer& GPUMesh::getVertexBuffer(const std::string& id)
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		assert(attribute != mAttributes.end());
		return *attribute->second;
	}


	IndexBuffer& GPUMesh::getOrCreateIndexBuffer(int index)
	{
		if (index < mIndexBuffers.size())
			return *mIndexBuffers[index];

		auto index_buffer = std::make_unique<IndexBuffer>(mRenderService->getCore(), mUsage);
		mIndexBuffers.emplace_back(std::move(index_buffer));
		return *mIndexBuffers.back();
	}


	const IndexBuffer& GPUMesh::getIndexBuffer(int index) const
	{
		assert(index < mIndexBuffers.size());
		return *mIndexBuffers[index];
	}


	template<typename ELEMENTTYPE>
	ValueGPUBuffer& GPUMesh::addVertexBuffer(const std::string& id)
	{
		auto vertex_buffer = std::make_unique<TypedValuePropertyGPUBuffer<ELEMENTTYPE, EValueGPUBufferProperty::Vertex>>(mRenderService->getCore(), mUsage);
		auto it = mAttributes.emplace(std::make_pair(id, std::move(vertex_buffer))).first;
		return *it->second;
	}


	// Explicit template instantiations
	template ValueGPUBuffer& GPUMesh::addVertexBuffer<uint>(const std::string&);
	template ValueGPUBuffer& GPUMesh::addVertexBuffer<int>(const std::string&);
	template ValueGPUBuffer& GPUMesh::addVertexBuffer<float>(const std::string&);
	template ValueGPUBuffer& GPUMesh::addVertexBuffer<glm::vec2>(const std::string&);
	template ValueGPUBuffer& GPUMesh::addVertexBuffer<glm::vec3>(const std::string&);
	template ValueGPUBuffer& GPUMesh::addVertexBuffer<glm::vec4>(const std::string&);
}
