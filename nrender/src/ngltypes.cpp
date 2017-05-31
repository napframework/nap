// Local Includes
#include "ngltypes.h"
#include "nglutils.h"

// External Includes
#include <unordered_map>

namespace opengl
{
	/**
	 * returns the map that binds GL type id's with their respected c++ data size
	 */
	using TypeMap = std::unordered_map<GLenum, std::size_t>;
	static const TypeMap& getTypeMap()
	{
		static TypeMap map;
		if (map.empty())
		{
			map[GL_BYTE]			= sizeof(GLbyte);
			map[GL_UNSIGNED_BYTE]	= sizeof(GLubyte);
			map[GL_SHORT]			= sizeof(GLshort);
			map[GL_UNSIGNED_SHORT]  = sizeof(GLushort);
			map[GL_INT]				= sizeof(GLint);
			map[GL_UNSIGNED_INT]	= sizeof(GLuint);
			map[GL_HALF_FLOAT]		= sizeof(GLhalf);
			map[GL_FLOAT]			= sizeof(GLfloat);
			map[GL_DOUBLE]			= sizeof(GLdouble);
			map[GL_FIXED]			= sizeof(GLfixed);
		}
		return map;
	}


	/**
	 * Returns the data size in bytes associated
	 */
	std::size_t getGLTypeSize(GLenum type)
	{
		const TypeMap& map = getTypeMap();
		auto it = map.find(type);
		if (it == map.end())
		{
			printMessage(MessageType::ERROR, "unable to find GL type with id: %d", type);
			return 0;
		}
		return it->second;
	}

}