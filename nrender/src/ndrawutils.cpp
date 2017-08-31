// Local Includes
#include <rtti/typeinfo.h>
#include "ndrawutils.h"

// External Includes
#include <unordered_map>
#include <algorithm>

RTTI_BEGIN_ENUM(opengl::EDrawMode)
	RTTI_ENUM_VALUE(opengl::EDrawMode::UNKNOWN,			"Unknown"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::POINTS,			"Points"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::LINES,			"Lines"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::LINE_STRIP,		"LineStrip"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::LINE_LOOP,		"LineLoop"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::TRIANGLES,		"Triangles"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::TRIANGLE_STRIP,	"TriangleStrip"),
	RTTI_ENUM_VALUE(opengl::EDrawMode::TRIANGLE_FAN,	"TriangleFan")
RTTI_END_ENUM

namespace opengl
{
	using OpenGLDrawModeMap = std::unordered_map<EDrawMode, GLenum>;
	static const OpenGLDrawModeMap openGLDrawMap =
	{
		{ EDrawMode::POINTS,			GL_POINTS },
		{ EDrawMode::LINES,				GL_LINES  },
		{ EDrawMode::LINE_STRIP,		GL_LINE_STRIP},
		{ EDrawMode::LINE_LOOP,			GL_LINE_LOOP},
		{ EDrawMode::TRIANGLES,			GL_TRIANGLES},
		{ EDrawMode::TRIANGLE_STRIP,	GL_TRIANGLE_STRIP},
		{ EDrawMode::TRIANGLE_FAN,		GL_TRIANGLE_STRIP}
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