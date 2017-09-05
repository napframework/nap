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
		mMeshInstance = std::make_unique<MeshInstance>();

		int numVertices = 4;
		mMeshInstance->setNumVertices(numVertices);
		mMeshInstance->setDrawMode(opengl::EDrawMode::TRIANGLES);
		Vec3VertexAttribute& position_attribute		= mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
		Vec3VertexAttribute& normal_attribute		= mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName());
		Vec3VertexAttribute& uv_attribute			= mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
		Vec4VertexAttribute& color_attribute		= mMeshInstance->GetOrCreateAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));

		position_attribute.setData(plane_vertices, numVertices);
		normal_attribute.setData(plane_normals, numVertices);
		uv_attribute.setData(plane_uvs, numVertices);
		color_attribute.setData(plane_colors, numVertices);
		mMeshInstance->setIndices(plane_indices, 6);

		return mMeshInstance->init(errorState);
	}
};
