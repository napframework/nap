#include "ngpumesh.h"
#include "nvertexbuffer.h"
#include "nindexbuffer.h"
#include <assert.h>

namespace nap
{
	void GPUMesh::addVertexAttribute(const std::string& id, VkFormat format)
	{
		mAttributes.emplace(std::make_pair(id, std::make_unique<VertexAttributeBuffer>(format)));
	}


	const VertexAttributeBuffer* GPUMesh::findVertexAttributeBuffer(const std::string& id) const
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		if (attribute != mAttributes.end())
			return attribute->second.get();

		return nullptr;
	}


	VertexAttributeBuffer& GPUMesh::getVertexAttributeBuffer(const std::string& id)
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		assert(attribute != mAttributes.end());
		return *attribute->second;
	}


	IndexBuffer& GPUMesh::getOrCreateIndexBuffer(int index)
	{
		if (index < mIndexBuffers.size())
			return *mIndexBuffers[index];
		
		std::unique_ptr<IndexBuffer> index_buffer = std::make_unique<IndexBuffer>();
		mIndexBuffers.emplace_back(std::move(index_buffer));

		return *mIndexBuffers.back();
	}


	const IndexBuffer& GPUMesh::getIndexBuffer(int index) const
	{
		return *mIndexBuffers[index];
	}

}
