#pragma once

// External Includes
#include <stdint.h>
#include <GL/glew.h>
#include <unordered_map>

namespace opengl
{
	/**
	* Holds all available GL drawing modes
	*/
	enum class EDrawMode : uint8_t
	{
		POINTS = 1,						///< Interpret the vertex data as single points
		LINES = 2,						///< Interpret the vertex data as individual lines
		LINE_STRIP = 3,					///< Interpret the vertex data as a single connected line
		LINE_LOOP = 4,					///< Interpret the vertex data as a line where the first and last vertex are connected
		TRIANGLES = 5,					///< Interpret the vertex data as a set of triangles
		TRIANGLE_STRIP = 6,				///< Interpret the vertex data as a strip of triangles
		TRIANGLE_FAN = 7,				///< Interpret the vertex data as a fan of triangles
		UNKNOWN = 0,					///< Invalid vertex interpretation
	};


	/**
	 * Returns the DrawMode's associated OpenGL mode
	 * @param mode the internal DrawMode
	 * @return the OpenGL draw mode enum, GL_INVALID_ENUM if mode is unknown
	 */
	GLenum getGLMode(EDrawMode mode);


	/**
	 * Returns the DrawMode associated with the OpenGL defined drawmode
	 * @param mode the OpenGL draw mode
	 * @return the correct DrawMode, UNKNOWN if that draw mode is unknown or unsupported
	 */
	EDrawMode getDrawMode(GLenum mode);

} // opengl

namespace std
{
	template <>
	struct hash<opengl::EDrawMode>
	{
		size_t operator()(const opengl::EDrawMode& v) const
		{
			return hash<uint8_t>()(static_cast<uint8_t>(v));
		}
	};
}
