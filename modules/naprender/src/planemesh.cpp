#include "planemesh.h"
#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>

// All the plane vertices
static glm::vec3 plane_vertices[] =
{
	{ -0.5,	-0.5f,	0.0f },
	{ 0.5f,	-0.5f,	0.0f },
	{ -0.5f,	0.5f,	0.0f },
	{ 0.5f,	0.5f,	0.0f },
};

// All the plane uvs
static glm::vec3 plane_uvs[] =
{
	{ 0.0f,	0.0f,	0.0f },
	{ 1.0f,	0.0f,	0.0f },
	{ 0.0f,	1.0f,	0.0f },
	{ 1.0f,	1.0f,	0.0f },
};

// All the plane colors
static glm::vec4 plane_colors[] =
{
	{ 1.0f,	1.0f,	1.0f,	1.0f },
	{ 1.0f,	1.0f,	1.0f,	1.0f },
	{ 1.0f,	1.0f,	1.0f,	1.0f },
	{ 1.0f,	1.0f,	1.0f,	1.0f },
};

// All the plane normals
static glm::vec3 plane_normals[] =
{
	{ 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, 1.0 },
	{ 0.0, 0.0, 1.0 }
};

// Plane connectivity indices
static unsigned int plane_indices[] =
{
	0,1,3,
	0,3,2
};


RTTI_BEGIN_CLASS(nap::PlaneMesh)
RTTI_END_CLASS

namespace nap
{
	bool PlaneMesh::init(utility::ErrorState& errorState)
	{
		mNumVertices = 4;
		mDrawMode = opengl::EDrawMode::TRIANGLES;
		Vec3VertexAttribute& position_attribute	= GetOrCreateAttribute<glm::vec3>(Mesh::VertexAttributeIDs::PositionVertexAttr);
		Vec3VertexAttribute& normal_attribute		= GetOrCreateAttribute<glm::vec3>(Mesh::VertexAttributeIDs::NormalVertexAttr);
		Vec3VertexAttribute& uv_attribute			= GetOrCreateAttribute<glm::vec3>(nap::utility::stringFormat("%s%d", Mesh::VertexAttributeIDs::UVVertexAttr.c_str(), 0));
		Vec4VertexAttribute& color_attribute		= GetOrCreateAttribute<glm::vec4>(nap::utility::stringFormat("%s%d", Mesh::VertexAttributeIDs::ColorVertexAttr.c_str(), 0));

		position_attribute.setData(plane_vertices, mNumVertices);
		normal_attribute.setData(plane_normals, mNumVertices);
		uv_attribute.setData(plane_uvs, mNumVertices);
		color_attribute.setData(plane_colors, mNumVertices);
		setIndices(plane_indices, 6);

		return Mesh::init(errorState);
	}
};
