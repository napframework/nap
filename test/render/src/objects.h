#pragma once

// External Includes
#include <nmesh.h>

// Creates a cube
std::unique_ptr<opengl::Mesh> createCube(int vertex_idx, int color_idx, int uv_idx);

// Creates a triangle
std::unique_ptr<opengl::Mesh> createTriangle(int vertex_idx, int color_idx, int uv_idx);

// Creates a plane
std::unique_ptr<opengl::Mesh> createPlane(int vertex_idx, int color_idx, int uv_idx);
