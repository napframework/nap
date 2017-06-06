#include <planemeshresource.h>
#include "meshresource.h"
#include "material.h"

// All the plane vertices
static float plane_vertices[] =
{
	-0.5,	-0.5f,	0.0f,
	0.5f,	-0.5f,	0.0f,
	-0.5f,	0.5f,	0.0f,
	0.5f,	0.5f,	0.0f,
};

// All the plane uvs
static float plane_uvs[] =
{
	0.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	1.0f,	1.0f,	0.0f,
};

// All the plane colors
static float plane_colors[] =
{
	1.0f,	1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	1.0f,
};

// All the plane normals
static float plane_normals[] =
{
	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0,
	0.0, 0.0, 1.0
};

// Plane connectivity indices
static unsigned int plane_indices[] =
{
	0,1,3,
	0,3,2
};

static opengl::Mesh* createPlane()
{
	opengl::Mesh* plane_mesh = new opengl::Mesh(4, opengl::EDrawMode::TRIANGLES);
	plane_mesh->addVertexAttribute(opengl::Mesh::VertexAttributeIDs::PositionVertexAttr, 3, plane_vertices);
	plane_mesh->addVertexAttribute(opengl::Mesh::VertexAttributeIDs::NormalVertexAttr, 3, plane_normals);
	plane_mesh->addVertexAttribute(nap::utility::stringFormat("%s%d", opengl::Mesh::VertexAttributeIDs::UVVertexAttr.c_str(), 0), 3, plane_uvs);
	plane_mesh->addVertexAttribute(nap::utility::stringFormat("%s%d", opengl::Mesh::VertexAttributeIDs::ColorVertexAttr.c_str(), 0), 4, plane_colors);
	plane_mesh->setIndices(6, plane_indices);

	return plane_mesh;
}

//////////////////////////////////////////////////////////////////////////


RTTI_BEGIN_CLASS(nap::PlaneMeshResource)
	RTTI_PROPERTY("DummyProperty", &nap::PlaneMeshResource::mDummyProperty, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool PlaneMeshResource::init(utility::ErrorState& errorState)
	{
		mMesh.reset(createPlane());		
		return true;
	}
};
