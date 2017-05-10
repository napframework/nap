#include <planecomponent.h>
#include "meshresource.h"
#include "modelresource.h"

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
	plane_mesh->addVertexAttribute(opengl::VertexAttributeIDs::PositionVertexAttr, 3, plane_vertices);
	plane_mesh->addVertexAttribute(opengl::VertexAttributeIDs::NormalVertexAttr, 3, plane_normals);
	plane_mesh->addVertexAttribute(nap::stringFormat("%s%d", opengl::VertexAttributeIDs::UVVertexAttr.c_str(), 0), 3, plane_uvs);
	plane_mesh->addVertexAttribute(nap::stringFormat("%s%d", opengl::VertexAttributeIDs::ColorVertexAttr.c_str(), 0), 4, plane_colors);
	plane_mesh->setIndices(6, plane_indices);

	return plane_mesh;
}

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	PlaneComponent::PlaneComponent(Material& material)
	{
		ErrorState error_state;
		CustomMeshResource* mesh_resource = new CustomMeshResource();
		mesh_resource->mCustomMesh.reset(createPlane());

		bool success = mesh_resource->init(error_state);
		assert(success);

		mModelResource = new ModelResource();
		mModelResource->mMaterialResource = &material;
		mModelResource->mMeshResource = mesh_resource;

		success = mModelResource->init(error_state);
		assert(success);
	}
};

RTTI_DEFINE(nap::PlaneComponent)
