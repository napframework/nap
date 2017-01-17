#pragma once

// External Includes
#include <nvertexarrayobject.h>

// Creates a cube
void createCube(opengl::VertexArrayObject& cube, int vertex_idx, int color_idx, int uv_idx);

// Creates a triangle
void createTriangle(opengl::VertexArrayObject& triangle, int vertex_idx, int color_idx, int uv_idx);
