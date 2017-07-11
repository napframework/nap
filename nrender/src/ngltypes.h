#pragma once

// External Includes
#include <GL/glew.h>
#include <cstddef>

namespace opengl
{
	/**
	 * @return size in bytes associated with a specific opengl type
	 * 0 is returned when the type is not supported
	 */
	std::size_t getGLTypeSize(GLenum type);
}
