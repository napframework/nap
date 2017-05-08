#include "nmesh.h"
#include <assert.h>

namespace opengl
{
	const VertexAttributeID VertexAttributeIDs::PositionVertexAttr("Position");
	const VertexAttributeID VertexAttributeIDs::NormalVertexAttr("Normal");
	const VertexAttributeID VertexAttributeIDs::UVVertexAttr("UV");
	const VertexAttributeID VertexAttributeIDs::ColorVertexAttr("Color");

	// Constructor initializes object draw mode
	Mesh::Mesh(int numVertices) :
		mNumVertices(numVertices)
	{
	}

	void Mesh::addVertexAttribute(const VertexAttributeID& id, unsigned int components, const float* data)
	{
		Attribute attribute;
		attribute.mID = id;
		attribute.mData = std::make_unique<FloatVertexContainer>();

		// Copy our data
		bool success = attribute.mData->copyData(components, mNumVertices, data);
		assert(success);

		// Synchronize on success (data in CPU memory will be uploaded to GPU)
		attribute.mData->sync();

		mAttributes.emplace_back(std::move(attribute));
	}

	const VertexBuffer* Mesh::findVertexAttributeBuffer(const VertexAttributeID& id) const
	{
		for (const Attribute& attribute : mAttributes)
			if (attribute.mID == id)
				return attribute.mData->getVertexBuffer();

		return nullptr;
	}

	void Mesh::setIndices(unsigned int count, const unsigned int* data)
	{
		assert(count > 0);

		if (mIndices == nullptr)
		{
			mIndices = std::make_unique<IndexContainer>();
		}

		// Copy our data
		mIndices->copyData(count, data);

		// Synchronize on success (data in CPU memory will be uploaded to GPU)
		mIndices->sync();
	}
}