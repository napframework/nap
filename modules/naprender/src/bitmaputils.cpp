#include "bitmaputils.h"
#include "utility/errorstate.h"
#include <assert.h>
#include "gl/glew.h"
#include "ntexture2d.h"
#include "bitmap.h"

namespace nap
{
	// Returns the associated OpenGL system type based on the Bitmap's data type
	GLenum getGLType(Bitmap::EDataType type)
	{
		switch (type)
		{
		case Bitmap::EDataType::BYTE:
			return GL_UNSIGNED_BYTE;
		case Bitmap::EDataType::FLOAT:
			return GL_FLOAT;
		case Bitmap::EDataType::USHORT:
			return GL_UNSIGNED_SHORT;
		}

		assert(false);
		return GL_INVALID_ENUM;
	}


	// the GL associated internal format associated with the BitmapColorType
	GLint getGLInternalFormat(Bitmap::EChannels type, bool compressed /*= true*/)
	{
		switch (type)
		{
		case Bitmap::EChannels::R:
			return compressed ? GL_COMPRESSED_RED_RGTC1_EXT : GL_RED;
		case Bitmap::EChannels::RGB:
		case Bitmap::EChannels::BGR:
			return compressed ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGB;
		case Bitmap::EChannels::RGBA:
		case Bitmap::EChannels::BGRA:
			return compressed ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA;
		}
		assert(false);
		return -1;
	}


	// Returns the GL associated format associated with the bitmap's color type
	GLenum getGLFormat(Bitmap::EChannels type)
	{
		switch (type)
		{
		case Bitmap::EChannels::R:
			return GL_RED;
		case Bitmap::EChannels::RGB:
			return GL_RGB;
		case Bitmap::EChannels::RGBA:
			return GL_RGBA;
		case Bitmap::EChannels::BGR:
			return GL_BGR;
		case Bitmap::EChannels::BGRA:
			return GL_BGRA;
		}

		assert(false);
		return GL_INVALID_ENUM;
	}


	// Populates a Texture2D object with settings matching the bitmap
	bool getTextureSettingsFromPixmap(const Bitmap& pixmap, bool compress, opengl::Texture2DSettings& settings, nap::utility::ErrorState& errorState)
	{
		// Fetch matching values
		GLint internal_format = getGLInternalFormat(pixmap.getChannels(), compress);
		if (!errorState.check(internal_format != GL_INVALID_VALUE, "Unable to determine internal format from bitmap"))
			return false;

		GLenum format = getGLFormat(pixmap.getChannels());
		if (!errorState.check(format != GL_INVALID_ENUM, "Unable to determine format from bitmap"))
			return false;

		GLenum type = getGLType(pixmap.getDataType());
		if (!errorState.check(type != GL_INVALID_ENUM, "Unable to determine texture type from bitmap"))
			return false;

		// Populate settings with fetched values from bitmap
		settings.mInternalFormat = internal_format;
		settings.mFormat = format;
		settings.mType = type;
		settings.mWidth = pixmap.getWidth();
		settings.mHeight = pixmap.getHeight();

		return true;
	}
}