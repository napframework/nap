#pragma once

// External Includes
#include <stdint.h>
#include <gl/glew.h>

namespace opengl
{
	/**
	* Holds all available GL drawing modes
	*/
	enum class DrawMode : uint8_t
	{
		POINTS = 1,
		LINES = 2,
		LINE_STRIP = 3,
		LINE_LOOP = 4,
		TRIANGLES = 5,
		TRIANGLE_STRIP = 6,
		TRIANGLE_FAN = 7,
		UNKNOWN = 0,
	};


	/**
	 * Returns the DrawMode's associated OpenGL mode
	 * @param mode the internal DrawMode
	 * @return the OpenGL draw mode enum, GL_INVALID_ENUM if mode is unknown
	 */
	GLenum getGLMode(DrawMode mode);


	/**
	 * Returns the DrawMode associated with the OpenGL defined drawmode
	 * @param mode the OpenGL draw mode
	 * @return the correct DrawMode, UNKNOWN if that draw mode is unknown or unsupported
	 */
	DrawMode getDrawMode(GLenum mode);

} // opengl
