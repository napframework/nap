#pragma once

// Local includes
#include "bitmap.h"

// External includes
#include <string>
#include <GL/glew.h>
#include <unordered_map>
#include <FreeImage.h>

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}
}

namespace opengl
{
	struct Texture2DSettings;
	class Texture;

	// using directives
	using OpenGLTypeMap = std::unordered_map<BitmapDataType, GLenum>;
	using OpenGLFormatMap = std::unordered_map<BitmapColorType, GLenum>;

	/**
	* getGLType
	* @return: the associated OpenGL system type based on the Bitmap's data type, GL_INVALID_ENUM if not found
	*/
	NAPAPI GLenum	getGLType(BitmapDataType type);

	/**
	 * getGLInternalFormat
	 *
	 * @return: the GL associated internal format associated with the BitmapColorType
	 * @param: compressed, if the internal format should be a compatible compressed format
	 * If no compression is available, the default uncompressed format is returned 
	 * GL_INVALID_VALUE is returned if no format is available at all
	 */
	NAPAPI GLint		getGLInternalFormat(BitmapColorType type, bool compressed = false);

	/**
	 * getGLFormat
	 * @return: the GL associated format associated with the bitmap's color type
	 * returns GL_INVALID_ENUM if no format is available at all
	 * The format determines the composition of each element in the texture
	 */
	NAPAPI GLenum		getGLFormat(BitmapColorType type);

	/**
	 * setFromBitmap
	 *
	 * Populates a Texture2D object with settings derived from the bitmap
	 * Note that this call does NOT initialize the texture object
	 * @return: If the Texture2D object is populated correctly
	 * @param bitmap: the bitmap to find matching Texture2D settings for
	 * @param texture: the texture to populate based on @bitmap
	 * @param compress: if the texture should be compressed when uploaded to the GPU  
	 */
	NAPAPI bool		getSettingsFromBitmap(const Bitmap& bitmap, bool compress, Texture2DSettings& settings, nap::utility::ErrorState& errorState);

	/**
	* @return a map that binds bitmap data types to opengl data types
	*/
	NAPAPI const OpenGLTypeMap& getGLTypeMap();

	/**
	*	@return a map that binds bitmap color types to opengl formats
	*/
	NAPAPI const OpenGLFormatMap& getGLFormatMap();

	/**
	 * getBitmapForFreeImageType
	 *
	 * Returns the associated Bitmap Data Type for the queried free image type
	 * @return: the associated bitmap type, UNKNOWN if not found
	 */
	NAPAPI BitmapDataType	getBitmapType(FREE_IMAGE_TYPE type);

	/**
	 * getBitmapColor
	 *
	 * Returns the associated Bitmap Color Type for the queried free image color type
	 * @return: the associated bitmap color, UNKNOWN if not found
	 */
	NAPAPI BitmapColorType getColorType(FREE_IMAGE_COLOR_TYPE colorType, FREE_IMAGE_TYPE dataType);

	/**
	 * @param type the opengl enum to get the associated bitmap type for
	 * @return the associated bitmap type for the queried opengl type, UNKNOWN if there is no valid conversion
	 */
	NAPAPI BitmapDataType getBitmapType(GLenum type);

	/**
	 * @param format the opengl texture format to get the associated color type for
	 * @return the associated bitmap color type for the queried opengl format
	 */
	NAPAPI BitmapColorType getColorType(GLenum format);

	/**
	 * loadBitmap
	 *
	 * Creates a bitmap from file, this bitmap owns the pixel data
	 * @return: a new bitmap, nullptr if unsuccessful
	 * @param imgPath: full path to image to load
	 */
	NAPAPI Bitmap*			loadBitmap(const std::string& imgPath, nap::utility::ErrorState& errorState);

	/**
	 * loadBitmap
	 *
	 * loads a bitmap from file, the bitmap owns the pixel data
	 * if the bitmap already contains data that data will be cleared!
	 * @param bitmap: the bitmap to set, clears existing data!
	 * @param imgPath: full path to image to load
	 * @return if the load was successful or not
	 */
	NAPAPI bool			loadBitmap(Bitmap& bitmap, const std::string& imgPath, nap::utility::ErrorState& errorState);
}


namespace std
{
    template<>
    struct hash<FREE_IMAGE_COLOR_TYPE>
    {
        size_t operator()(const FREE_IMAGE_COLOR_TYPE &v) const
        {
            return hash<int>()(static_cast<int>(v));
        }
    };
}
