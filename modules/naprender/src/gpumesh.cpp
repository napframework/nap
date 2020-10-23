/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gpumesh.h"
#include <assert.h>

namespace nap
{
	GPUMesh::GPUMesh(RenderService& renderService, EMeshDataUsage inUsage) :
		mRenderService(&renderService),
		mUsage(inUsage)
	{ }


	void GPUMesh::addVertexBuffer(const std::string& id, VkFormat format)
	{
		mAttributes.emplace(std::make_pair(id, std::make_unique<VertexBuffer>(*mRenderService, format, mUsage)));
	}


	const VertexBuffer* GPUMesh::findVertexBuffer(const std::string& id) const
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		if (attribute != mAttributes.end())
			return attribute->second.get();
		return nullptr;
	}


	VertexBuffer& GPUMesh::getVertexBuffer(const std::string& id)
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		assert(attribute != mAttributes.end());
		return *attribute->second;
	}


	IndexBuffer& GPUMesh::getOrCreateIndexBuffer(int index)
	{
		if (index < mIndexBuffers.size())
			return *mIndexBuffers[index];
		
		std::unique_ptr<IndexBuffer> index_buffer = std::make_unique<IndexBuffer>(*mRenderService, mUsage);
		mIndexBuffers.emplace_back(std::move(index_buffer));
		return *mIndexBuffers.back();
	}


	const IndexBuffer& GPUMesh::getIndexBuffer(int index) const
	{
		return *mIndexBuffers[index];
	}

}
