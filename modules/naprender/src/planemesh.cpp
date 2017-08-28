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

static opengl::Mesh* createPlane()
{
// 	opengl::Mesh* plane_mesh = new opengl::Mesh(4, opengl::EDrawMode::TRIANGLES);
// 	plane_mesh->addVertexAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::PositionVertexAttr, plane_vertices);
// 	plane_mesh->addVertexAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::NormalVertexAttr, plane_normals);
// 	plane_mesh->addVertexAttribute<glm::vec3>(nap::utility::stringFormat("%s%d", opengl::Mesh::VertexAttributeIDs::UVVertexAttr.c_str(), 0), plane_uvs);
// 	plane_mesh->addVertexAttribute<glm::vec4>(nap::utility::stringFormat("%s%d", opengl::Mesh::VertexAttributeIDs::ColorVertexAttr.c_str(), 0), plane_colors);
// 	plane_mesh->setIndices(6, plane_indices);
// 
// 	return plane_mesh;

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////


RTTI_BEGIN_CLASS(nap::PlaneMesh)
RTTI_END_CLASS

namespace nap
{
	bool PlaneMesh::init(utility::ErrorState& errorState)
	{
		//mMesh.reset(createPlane());		
		return true;
	}
};
