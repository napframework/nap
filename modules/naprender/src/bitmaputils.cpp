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