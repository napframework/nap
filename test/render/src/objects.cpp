// Local Includes
#include "objects.h"

// External Includes
#include <nvertexcontainer.h>
#include <nindexbuffer.h>
#include "nmesh.h"
#include <memory>

// Cube vertices
static float cube_vertices[] =
{
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,

	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,

	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
};

// Cube Colors
static float cube_colors[]
{
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,

	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,

	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 1.0f, 1.0f,

	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f, 1.0f,

	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,

	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f
};


// Cube Uv's
static float cube_uvs[] =
{
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f,	0.0f,
	1.0f, 1.0f,	1.0f,
	1.0f, 1.0f,	1.0f,
	0.0f, 1.0f,	1.0f,
	0.0f, 0.0f,	0.0f,

	0.0f, 0.0f,	0.0f,
	1.0f, 0.0f,	0.0f,
	1.0f, 1.0f,	1.0f,
	1.0f, 1.0f,	1.0f,
	0.0f, 1.0f,	1.0f,
	0.0f, 0.0f,	0.0f,

	1.0f, 0.0f,	0.0f,
	1.0f, 1.0f,	1.0f,
	0.0f, 1.0f,	1.0f,
	0.0f, 1.0f,	1.0f,
	0.0f, 0.0f,	0.0f,
	1.0f, 0.0f,	0.0f,

	1.0f, 0.0f,	0.0f,
	1.0f, 1.0f,	1.0f,
	0.0f, 1.0f,	1.0f,
	0.0f, 1.0f,	1.0f,
	0.0f, 0.0f,	0.0f,
	1.0f, 0.0f,	0.0f,

	0.0f, 1.0f,	1.0f,
	1.0f, 1.0f,	1.0f,
	1.0f, 0.0f,	0.0f,
	1.0f, 0.0f,	0.0f,
	0.0f, 0.0f,	0.0f,
	0.0f, 1.0f,	1.0f,

	0.0f, 1.0f,	1.0f,
	1.0f, 1.0f,	1.0f,
	1.0f, 0.0f,	0.0f,
	1.0f, 0.0f,	0.0f,
	0.0f, 0.0f,	0.0f,
	0.0f, 1.0f,	1.0f,
};


static float triangle_vertices[] =
{
	-1.0f,	-1.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	1.0f,	-1.0f,	0.0f
};


static float triangle_colors[] =
{
	1.0f,	0.0f,	0.0f,  1.0f,
	0.0f,	1.0f,	0.0f,  1.0f,
	0.0f,	0.0f,	1.0f , 1.0f
};


static float triangle_uvs[] =
{
	1.0f,	1.0f, 1.0f,
	1.0f,	0.0f, 0.0f,
	0.0f,	0.0f, 0.0f
};

static unsigned int triangle_indices[] =
{
	0,1,2
};

static float plane_vertices[] =
{
	-1.0,	-1.0f,	0.0f,
	1.0f,	-1.0f,	0.0f,
	-1.0f,	1.0f,	0.0f,
	1.0f,	1.0f,	0.0f,
};

static float plane_uvs[] =
{
	0.0f,	0.0f,	0.0f,
	1.0f,	0.0f,	0.0f,
	0.0f,	1.0f,	0.0f,
	1.0f,	1.0f,	0.0f,
};

static float plane_colors[] =
{
	1.0f,	1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	1.0f,
	1.0f,	1.0f,	1.0f,	1.0f,
};

static unsigned int plane_indices[] =
{
	0,1,3,
	0,3,2
};




// Creates a cube
std::unique_ptr<opengl::Mesh> createCube(int vertex_idx, int color_idx, int uv_idx)
{
	std::unique_ptr<opengl::Mesh> mesh = std::make_unique<opengl::Mesh>(6, opengl::EDrawMode::TRIANGLES);

	mesh->addVertexAttribute(opengl::VertexAttributeIDs::PositionVertexAttr, 3, cube_vertices);
	mesh->addVertexAttribute(opengl::VertexAttributeIDs::GetColorVertexAttr(0), 4, cube_colors);
	mesh->addVertexAttribute(opengl::VertexAttributeIDs::GetUVVertexAttr(0), 3, cube_uvs);

	return mesh;
}


// Creates a simple triangle
std::unique_ptr<opengl::Mesh> createTriangle(int vertex_idx, int color_idx, int uv_idx)
{
	std::unique_ptr<opengl::Mesh> mesh = std::make_unique<opengl::Mesh>(3, opengl::EDrawMode::TRIANGLES);

	mesh->addVertexAttribute(opengl::VertexAttributeIDs::PositionVertexAttr, 3, triangle_vertices);
	mesh->addVertexAttribute(opengl::VertexAttributeIDs::GetColorVertexAttr(0), 4, triangle_colors);
	mesh->addVertexAttribute(opengl::VertexAttributeIDs::GetUVVertexAttr(0), 3, triangle_uvs);
	mesh->setIndices(3, triangle_indices);

	return mesh;
}

std::unique_ptr<opengl::Mesh> createPlane(int vertex_idx, int color_idx, int uv_idx)
{
	std::unique_ptr<opengl::Mesh> mesh = std::make_unique<opengl::Mesh>(4, opengl::EDrawMode::TRIANGLES);

	mesh->addVertexAttribute(opengl::VertexAttributeIDs::PositionVertexAttr, 3, plane_vertices);
	mesh->addVertexAttribute(opengl::VertexAttributeIDs::GetColorVertexAttr(0), 4, plane_colors);
	mesh->addVertexAttribute(opengl::VertexAttributeIDs::GetUVVertexAttr(0), 3, plane_uvs);
	mesh->setIndices(6, plane_indices);

	return mesh;
}
