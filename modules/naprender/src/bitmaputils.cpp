#include "bitmaputils.h"
#include "utility/errorstate.h"
#include <assert.h>
#include "GL/glew.h"
#include "ntexture2d.h"
#include "bitmap.h"

namespace nap
{
	// Returns the associated OpenGL system type based on the Bitmap's data type
	GLenum getGLType(ESurfaceDataType type)
	{
		switch (type)
		{
		case ESurfaceDataType::BYTE:
			return GL_UNSIGNED_BYTE;
		case ESurfaceDataType::FLOAT:
			return GL_FLOAT;
		case ESurfaceDataType::USHORT:
			return GL_UNSIGNED_SHORT;
		}

		assert(false);
		return GL_INVALID_ENUM;
	}


	// the GL associated internal format associated with the BitmapColorType
	GLint getGLInternalFormat(ESurfaceChannels type, bool compressed /*= true*/)
	{
		switch (type)
		{
		case ESurfaceChannels::R:
			return compressed ? GL_COMPRESSED_RED_RGTC1_EXT : GL_RED;
		case ESurfaceChannels::RGB:
		case ESurfaceChannels::BGR:
			return compressed ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGB;
		case ESurfaceChannels::RGBA:
		case ESurfaceChannels::BGRA:
			return compressed ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA;
		}
		assert(false);
		return -1;
	}


	// Returns the GL associated format associated with the bitmap's color type
	GLenum getGLFormat(ESurfaceChannels type)
	{
		switch (type)
		{
		case ESurfaceChannels::R:
			return GL_RED;
		case ESurfaceChannels::RGB:
			return GL_RGB;
		case ESurfaceChannels::RGBA:
			return GL_RGBA;
		case ESurfaceChannels::BGR:
			return GL_BGR;
		case ESurfaceChannels::BGRA:
			return GL_BGRA;
		}

		assert(false);
		return GL_INVALID_ENUM;
	}


	// Populates a Texture2D object with settings matching the bitmap
	bool getTextureSettingsFromBitmap(const Bitmap& bitmap, bool compress, Texture2DSettings& settings, nap::utility::ErrorState& errorState)
	{
		assert(false);
		return false;
// 		// Fetch matching values
// 		GLint internal_format = getGLInternalFormat(bitmap.getChannels(), compress);
// 		if (!errorState.check(internal_format != GL_INVALID_VALUE, "Unable to determine internal format from bitmap"))
// 			return false;
// 
// 		GLenum format = getGLFormat(bitmap.getChannels());
// 		if (!errorState.check(format != GL_INVALID_ENUM, "Unable to determine format from bitmap"))
// 			return false;
// 
// 		GLenum type = getGLType(bitmap.getDataType());
// 		if (!errorState.check(type != GL_INVALID_ENUM, "Unable to determine texture type from bitmap"))
// 			return false;
// 
// 		// Populate settings with fetched values from bitmap
// 		settings.mInternalFormat = internal_format;
// 		settings.mFormat = format;
// 		settings.mType = type;
// 		settings.mWidth = bitmap.getWidth();
// 		settings.mHeight = bitmap.getHeight();
// 
// 		return true;
	}
}