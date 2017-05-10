// Local Includes
#include "ndrawutils.h"

// External Includes
#include <unordered_map>
#include <algorithm>

namespace opengl
{
	using OpenGLDrawModeMap = std::unordered_map<EDrawMode, GLenum>;
	static const OpenGLDrawModeMap openGLDrawMap =
	{
		{ EDrawMode::POINTS,			GL_POINTS },
		{ EDrawMode::LINES,			GL_LINES  },
		{ EDrawMode::LINE_STRIP,		GL_LINE_STRIP},
		{ EDrawMode::LINE_LOOP,		GL_LINE_LOOP},
		{ EDrawMode::TRIANGLES,		GL_TRIANGLES},
		{ EDrawMode::TRIANGLE_STRIP,	GL_TRIANGLE_STRIP},
		{ EDrawMode::TRIANGLE_FAN,	GL_TRIANGLE_STRIP}
	};


	// Returns the DrawMode's OpenGL associated mode
	GLenum getGLMode(EDrawMode mode)
	{
		auto it = openGLDrawMap.find(mode);
		return it == openGLDrawMap.end() ? GL_INVALID_ENUM : it->second;
	}


	// Returns the DrawMode associated with the OpenGL defined drawmode
	EDrawMode getDrawMode(GLenum mode)
	{
		auto found_it = std::find_if(openGLDrawMap.begin(), openGLDrawMap.end(), [&](const auto& value)
		{
			return value.second == mode;
		});

		return found_it == openGLDrawMap.end() ? EDrawMode::UNKNOWN : (*found_it).first;
	}
}