#include "spheremesh.h"
#include "math.h"
#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>


/**
 * Creates an opengl sphere with it's associated CPU data
 * This call will initialize the sphere as well so an active gl context 
 * needs to be available
 */
static opengl::Mesh* createSphere(float radius, unsigned int rings, unsigned int sectors)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> texcoords;
	std::vector<glm::vec4> colors;
	std::vector<unsigned int> indices;

	float const R = 1. / (float)(rings - 1);
	float const S = 1. / (float)(sectors - 1);
	int r, s;

	// Get total amount of vertices
	unsigned int vertex_count = rings * sectors;

	vertices.resize(vertex_count);
	normals.resize(vertex_count);
	texcoords.resize(vertex_count);
	colors.resize(vertex_count);

	std::vector<glm::vec3>::iterator v = vertices.begin();
	std::vector<glm::vec3> ::iterator n = normals.begin();
	std::vector<glm::vec3>::iterator t = texcoords.begin();
	std::vector<glm::vec4>::iterator c = colors.begin();

	for (r = 0; r < rings; r++)
	{
		for (s = 0; s < sectors; s++)
		{
			float const y = sin(-(M_PI / 2.0) + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

			// Set texture coordinates
			*t++ = { 1.0f - (s*S), r*R, 0.5f };

			// Set vertex coordinates
			*v++ = { x * radius, y * radius, z * radius };

			// Set normal coordinates
			*n++ = { x, y, z };

			// Set color coordinates
			*c++ = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
	}

	// Calculate sphere indices
	int irings = rings - 1;
	int isectors = sectors - 1;
	unsigned int index_count = irings * isectors * 6;
	indices.resize(index_count);
	std::vector<unsigned int>::iterator i = indices.begin();
	for (r = 0; r < irings; r++)
	{
		for (s = 0; s < isectors; s++)
		{
			// Triangle A
			*i++ = (r * sectors) + s;
			*i++ = (r * sectors) + (s + 1);
			*i++ = ((r + 1) * sectors) + (s + 1);

			// Triangle B
			*i++ = (r * sectors) + s;
			*i++ = ((r + 1) * sectors) + (s + 1);
			*i++ = ((r + 1) * sectors) + s;
		}
	}

	opengl::Mesh* sphere_mesh = new opengl::Mesh(vertex_count, opengl::EDrawMode::TRIANGLES);
	sphere_mesh->addVertexAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::PositionVertexAttr, &vertices.front());
	sphere_mesh->addVertexAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::NormalVertexAttr,  &normals.front());
	sphere_mesh->addVertexAttribute<glm::vec3>(nap::utility::stringFormat("%s%d", opengl::Mesh::VertexAttributeIDs::UVVertexAttr.c_str(), 0), &texcoords.front());
	sphere_mesh->addVertexAttribute<glm::vec4>(nap::utility::stringFormat("%s%d", opengl::Mesh::VertexAttributeIDs::ColorVertexAttr.c_str(), 0), &colors.front());
	sphere_mesh->setIndices(index_count, &indices.front());
	
	return sphere_mesh;
}

RTTI_BEGIN_CLASS(nap::SphereMesh)
	RTTI_PROPERTY("Radius",		&nap::SphereMesh::mRadius,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rings",		&nap::SphereMesh::mRings,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sectors",	&nap::SphereMesh::mSectors, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool SphereMesh::init(utility::ErrorState& errorState)
	{
		mMesh.reset(createSphere(mRadius, mRings, mSectors));
		return true;
	}
}
