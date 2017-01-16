// Local Includes
#include "ndrawutils.h"

// External Includes
#include <unordered_map>
#include <algorithm>

namespace opengl
{
	using OpenGLDrawModeMap = std::unordered_map<DrawMode, GLenum>;
	static const OpenGLDrawModeMap openGLDrawMap =
	{
		{ DrawMode::POINTS,			GL_POINTS },
		{ DrawMode::LINES,			GL_LINES  },
		{ DrawMode::LINE_STRIP,		GL_LINE_STRIP},
		{ DrawMode::LINE_LOOP,		GL_LINE_LOOP},
		{ DrawMode::TRIANGLES,		GL_TRIANGLES},
		{ DrawMode::TRIANGLE_STRIP,	GL_TRIANGLE_STRIP},
		{ DrawMode::TRIANGLE_FAN,	GL_TRIANGLE_STRIP}
	};


	// Returns the DrawMode's OpenGL associated mode
	GLenum getGLMode(DrawMode mode)
	{
		auto it = openGLDrawMap.find(mode);
		return it == openGLDrawMap.end() ? GL_INVALID_ENUM : it->second;
	}


	// Returns the DrawMode associated with the OpenGL defined drawmode
	DrawMode getDrawMode(GLenum mode)
	{
		auto found_it = std::find_if(openGLDrawMap.begin(), openGLDrawMap.end(), [&](const auto& value)
		{
			return value.second == mode;
		});

		return found_it == openGLDrawMap.end() ? DrawMode::UNKNOWN : (*found_it).first;
	}
}