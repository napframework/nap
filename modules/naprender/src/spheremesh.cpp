#include "spheremesh.h"
#include "math.h"
#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>


RTTI_BEGIN_CLASS(nap::SphereMesh)
	RTTI_PROPERTY("Radius",		&nap::SphereMesh::mRadius,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rings",		&nap::SphereMesh::mRings,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sectors",	&nap::SphereMesh::mSectors, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool SphereMesh::init(utility::ErrorState& errorState)
	{
		initSphere();
		return Mesh::init(errorState);
	}

	void SphereMesh::initSphere()
	{
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> texcoords;
		std::vector<glm::vec4> colors;
		std::vector<unsigned int> indices;

		float const R = 1. / (float)(mRings - 1);
		float const S = 1. / (float)(mSectors - 1);
		int r, s;

		// Get total amount of vertices
		unsigned int vertex_count = mRings * mSectors;

		vertices.resize(vertex_count);
		normals.resize(vertex_count);
		texcoords.resize(vertex_count);
		colors.resize(vertex_count);

		std::vector<glm::vec3>::iterator v = vertices.begin();
		std::vector<glm::vec3> ::iterator n = normals.begin();
		std::vector<glm::vec3>::iterator t = texcoords.begin();
		std::vector<glm::vec4>::iterator c = colors.begin();

		for (r = 0; r < mRings; r++)
		{
			for (s = 0; s < mSectors; s++)
			{
				float const y = sin(-(M_PI / 2.0) + M_PI * r * R);
				float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
				float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

				// Set texture coordinates
				*t++ = { 1.0f - (s*S), r*R, 0.5f };

				// Set vertex coordinates
				*v++ = { x * mRadius, y * mRadius, z * mRadius };

				// Set normal coordinates
				*n++ = { x, y, z };

				// Set color coordinates
				*c++ = { 1.0f, 1.0f, 1.0f, 1.0f };
			}
		}

		// Calculate sphere indices
		int irings = mRings - 1;
		int isectors = mSectors - 1;
		unsigned int index_count = irings * isectors * 6;
		indices.resize(index_count);
		std::vector<unsigned int>::iterator i = indices.begin();
		for (r = 0; r < irings; r++)
		{
			for (s = 0; s < isectors; s++)
			{
				// Triangle A
				*i++ = (r * mSectors) + s;
				*i++ = (r * mSectors) + (s + 1);
				*i++ = ((r + 1) * mSectors) + (s + 1);

				// Triangle B
				*i++ = (r * mSectors) + s;
				*i++ = ((r + 1) * mSectors) + (s + 1);
				*i++ = ((r + 1) * mSectors) + s;
			}
		}

		mNumVertices = vertex_count;
		mDrawMode = opengl::EDrawMode::TRIANGLES;
		nap::Vec3VertexAttribute& position_attribute = GetOrCreateAttribute<glm::vec3>(nap::Mesh::VertexAttributeIDs::PositionVertexAttr);
		nap::Vec3VertexAttribute& normal_attribute = GetOrCreateAttribute<glm::vec3>(nap::Mesh::VertexAttributeIDs::NormalVertexAttr);
		nap::Vec3VertexAttribute& uv_attribute = GetOrCreateAttribute<glm::vec3>(nap::utility::stringFormat("%s%d", nap::Mesh::VertexAttributeIDs::UVVertexAttr.c_str(), 0));
		nap::Vec4VertexAttribute& color_attribute = GetOrCreateAttribute<glm::vec4>(nap::utility::stringFormat("%s%d", nap::Mesh::VertexAttributeIDs::ColorVertexAttr.c_str(), 0));

		position_attribute.setData(vertices.data(), mNumVertices);
		normal_attribute.setData(normals.data(), mNumVertices);
		uv_attribute.setData(texcoords.data(), mNumVertices);
		color_attribute.setData(colors.data(), mNumVertices);
		setIndices(indices.data(), index_count);
	}
}
