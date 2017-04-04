// Local Includes
#include "objects.h"

// External Includes
#include <nvertexcontainer.h>
#include <nindexbuffer.h>

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
void createCube(opengl::VertexArrayObject& cube, int vertex_idx, int color_idx, int uv_idx)
{
	static opengl::FloatVertexBuffer squarePositionBuffer;
	static opengl::FloatVertexBuffer squareColorBuffer;
	static opengl::FloatVertexBuffer squareUVBuffer;
	static bool buffersAllocated = false;

	if (!buffersAllocated)
	{
		// Get number of points
		int face_point_count(6);
		int face_count(6);
		int point_count = face_point_count * face_count;

		// Get colors
		int color_size = 4;
		int color_count = color_size * point_count;

		// Get number of vertices
		int vert_size = 3;
		int vert_count = vert_size * point_count;

		// Get number of uv's
		int uv_size = 3;
		int uv_count = uv_size * point_count;

		squarePositionBuffer.init();
		squarePositionBuffer.setData(vert_size, point_count, GL_STATIC_DRAW, cube_vertices);

		squareColorBuffer.init();
		squareColorBuffer.setData(color_size, point_count, GL_STATIC_DRAW, cube_colors);

		squareUVBuffer.init();
		squareUVBuffer.setData(uv_size, point_count, GL_STATIC_DRAW, cube_uvs);

		buffersAllocated = true;
	}

	// Init container and set draw mode
	cube.setDrawMode(opengl::DrawMode::TRIANGLES);

	// Add buffers to cube
	cube.addVertexBuffer(vertex_idx, squarePositionBuffer);
	cube.addVertexBuffer(color_idx, squareColorBuffer);
	cube.addVertexBuffer(uv_idx, squareUVBuffer);
}


// Creates a simple triangle
void createTriangle(opengl::VertexArrayObject& triangle, int vertex_idx, int color_idx, int uv_idx)
{
	// Vertex containers
	static opengl::VertexContainer trianglePositions;
	static opengl::VertexContainer triangleColors;
	static opengl::VertexContainer triangleUvs;
	static opengl::IndexBuffer triangleIndexBuffer;
	static bool buffersAllocated = false;

	if (!buffersAllocated)
	{
		int face_point_count = 3;
		int face_count = 1;
		int point_count = face_point_count * face_count;
		int vert_size = 3;
		int color_size = 4;
		int uv_size = 3;

		// Copy over triangle data in container
		trianglePositions.copyData(GL_FLOAT, vert_size, point_count, triangle_vertices);
		triangleColors.copyData(GL_FLOAT, color_size, point_count, triangle_colors);
		triangleUvs.copyData(GL_FLOAT, uv_size, point_count, triangle_uvs);

		triangleIndexBuffer.init();
		triangleIndexBuffer.setData(triangle_indices, 3);

		buffersAllocated = true;
	}

	triangle.setDrawMode(opengl::DrawMode::TRIANGLES);

	triangle.addVertexBuffer(vertex_idx, *trianglePositions.getVertexBuffer());
	triangle.addVertexBuffer(color_idx, *triangleColors.getVertexBuffer());
	triangle.addVertexBuffer(uv_idx, *triangleUvs.getVertexBuffer());
	
	// Use indices for triangle
	triangle.setIndexBuffer(triangleIndexBuffer);
}

void createPlane(opengl::VertexArrayObject& plane, int vertex_idx, int color_idx, int uv_idx)
{
	static opengl::VertexContainer planePositions;
	static opengl::VertexContainer planeColors;
	static opengl::VertexContainer planeUvs;
	static opengl::IndexBuffer planeIndexBuffer;
	static bool buffersAllocated = false;

	if (!buffersAllocated)
	{
		int point_count = 4;
		int vert_size = 3;
		int color_size = 4;
		int uv_size = 3;

		// Copy over triangle data in container
		planePositions.copyData(GL_FLOAT, vert_size, point_count, plane_vertices);
		planeColors.copyData(GL_FLOAT, color_size, point_count, plane_colors);
		planeUvs.copyData(GL_FLOAT, uv_size, point_count, plane_uvs);

		planeIndexBuffer.init();
		planeIndexBuffer.setData(plane_indices, 6);
		buffersAllocated = true;
	}


	plane.setDrawMode(opengl::DrawMode::TRIANGLES);

	plane.addVertexBuffer(vertex_idx, *planePositions.getVertexBuffer());
	plane.addVertexBuffer(color_idx, *planeColors.getVertexBuffer());
	plane.addVertexBuffer(uv_idx, *planeUvs.getVertexBuffer());

	// Use indices for triangle
	plane.setIndexBuffer(planeIndexBuffer);
}
