#include <planecomponent.h>

// The plane
static std::unique_ptr<opengl::Mesh> sPlane = nullptr;

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

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	opengl::Mesh* PlaneComponent::getMesh() const
	{
		// TODO: Make Thread Safe
		if (sPlane == nullptr)
		{
			sPlane = std::make_unique<opengl::Mesh>();
			sPlane->init();
			sPlane->copyVertexData(4, plane_vertices);
			sPlane->copyNormalData(4, plane_normals);
			sPlane->copyUVData(3, 4, plane_uvs);
			sPlane->copyColorData(4, 4, plane_colors);
			sPlane->copyIndexData(6, plane_indices);
		}
		return sPlane.get();
	}
};

RTTI_DEFINE(nap::PlaneComponent)
