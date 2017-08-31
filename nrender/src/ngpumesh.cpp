#include "ngpumesh.h"
#include <assert.h>

namespace opengl
{
	void GPUMesh::addVertexAttribute(const VertexAttributeID& id, GLenum type, unsigned int numComponents, unsigned int numVertices, GLenum usage)
	{
		mAttributes.emplace(std::make_pair(id, std::move(std::make_unique<VertexAttributeBuffer>(type, numComponents, numVertices, usage))));
	}


	const VertexAttributeBuffer* GPUMesh::findVertexAttributeBuffer(const VertexAttributeID& id) const
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		if (attribute != mAttributes.end())
			return attribute->second.get();

		return nullptr;
	}


	const VertexAttributeBuffer& GPUMesh::getVertexAttributeBuffer(const VertexAttributeID& id) const
	{
		AttributeMap::const_iterator attribute = mAttributes.find(id);
		assert(attribute != mAttributes.end());
		return *attribute->second;
	}


	opengl::IndexBuffer& GPUMesh::getOrCreateIndexBuffer()
	{
		if (mIndexBuffer == nullptr)
			mIndexBuffer = std::make_unique<IndexBuffer>();

		return *mIndexBuffer;
	}


	const opengl::IndexBuffer* GPUMesh::getIndexBuffer() const
	{
		return mIndexBuffer.get();
	}

}
