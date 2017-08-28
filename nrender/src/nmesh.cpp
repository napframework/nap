#include "nmesh.h"
#include <assert.h>

namespace opengl
{
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::PositionVertexAttr("Position");
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::NormalVertexAttr("Normal");
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::UVVertexAttr("UV");
	const Mesh::VertexAttributeID Mesh::VertexAttributeIDs::ColorVertexAttr("Color");

	const opengl::Mesh::VertexAttributeID Mesh::VertexAttributeIDs::GetUVVertexAttr(int uvChannel)
	{
		std::ostringstream stream;
		stream << UVVertexAttr << uvChannel;
		return stream.str();
	}


	const opengl::Mesh::VertexAttributeID Mesh::VertexAttributeIDs::GetColorVertexAttr(int colorChannel)
	{
		std::ostringstream stream;
		stream << ColorVertexAttr << colorChannel;
		return stream.str();
	}

	void Mesh::addVertexAttribute(const VertexAttributeID& id, GLenum type, unsigned int numComponents, unsigned int numVertices, GLenum usage)
	{
		mAttributes.emplace_back(std::move(Attribute(id, type, numComponents, numVertices, usage)));
	}

	const VertexAttributeBuffer* Mesh::findVertexAttributeBuffer(const VertexAttributeID& id) const
	{
		for (const Attribute& attribute : mAttributes)
        {
			if (attribute.mID == id)
				return attribute.mBuffer.get();
        }

		return nullptr;
	}


	void Mesh::setIndices(unsigned int count, const unsigned int* data)
	{
		assert(count > 0);

		// Copy our data
		//mIndices.copyData(count, data);

		// Synchronize on success (data in CPU memory will be uploaded to GPU)
		//mIndices->sync();
	}


	const opengl::IndexBuffer* Mesh::getIndexBuffer() const
	{
		return &mIndexBuffer;
	}

}
