#pragma once

#include "ntexture.h"
#include "nbitmap.h"

namespace opengl
{
	/**
	* Texture2Dsettings
	*
	* Data associated with a 2d texture
	*/
	struct Texture2DSettings
	{
	public:
		GLint internalFormat = GL_RGB;		//< Specifies the number of color components in the texture
		GLsizei width = 0;					//< Specifies the width of the texture
		GLsizei height = 0;					//< Specifies the height of the texture
		GLenum format = GL_BGR;				//< Specifies the format of the pixel data
		GLenum type = GL_UNSIGNED_BYTE;		//< Data type of the pixel data (GL_UNSIGNED_BYTE etc..)
	};

	/**
	 * Texture2D
	 *
	 * Represents a 2 dimensional texture on the GPU
	 */
	class Texture2D : public Texture
	{
	public:
		// Default constructor
		Texture2D();

		void init(const Texture2DSettings& textureSettings, const TextureParameters& parameters, ETextureUsage usage);

		/**
		 * @return Texture2D settings object
		 */
		const Texture2DSettings& getSettings() const { return mSettings; }

		/**
		 * setData
		 * 
		 * Uploads 2d pixel data to the GPU
		 * Make sure that the data the pointer points at matches the size of the texture settings provided!
		 *
		 * @param data Pointer to the appropriately-sized buffer to copy to the texture
		 * @param pitch The size (in bytes) of each row of pixels. The default of 0 means that it will use the internal opengl format to determine this.
		 */
		void setData(void* data, int pitch = 0);

		/**
		 * @return The size of the texture when copied to/from CPU.
		 */
		int getDataSize() const;

		/**
		 * Blocking call to retrieve GPU texture data. 
		 * @param data Block of data that is filled with texture data. The vector is resized internally to the correct size.
		 */
		void getData(std::vector<uint8_t>& data);

		/**
		 * Blocking call to retrieve GPU texture data that is stored in the bitmap
		 * When the bitmap is empty (has no data associated with it) this call will try
		 * to initialize the bitmap using the settings associated with this texture. This call will assert
		 * if it can't initialize the bitmap based on the  provided settings or when the internal settings do not match
		 * To find valid bitmap settings based on this texture use: opengl::getBitmapType and opengl::getColorType
		 * @param bitmap the bitmap that is filled with texture data
		 */
		void getData(opengl::Bitmap& bitmap);

		/**
		 * Starts a transfer of texture data from GPU to CPU. Use asyncEndGetData to block waiting for the async command to complete.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered. It is recommended
		 * to use double or triple buffering to make sure that no stalls occur when calling asyncEndGetData().
		 */
		void asyncStartGetData();

		/**
		 * Finishes a transfer of texture data from GPU to CPU that was started with asyncStartGetData. See comment in asyncStartGetData for proper use.
		 * @param data Block of data that is filled with texture data. The vector is resized internally to the correct size.
		 */
		void asyncEndGetData(std::vector<uint8_t>& data);

		/**
		 * Finishes a transfer of texture data from GPU to CPU that was started with asyncStartGetData. See comment in asyncStartGetData for proper use.
		 * @param bitmap the bitmap that is filled with texture data, see getData() for more documentation
		 */
		void asyncEndGetData(opengl::Bitmap& bitmap);

	private:
		Texture2DSettings	mSettings;		// Settings object
		GLuint				mPBO;			// Pixel buffer object used to read/write texture data if usage is DynamicRead or DynamicWrite
		ETextureUsage		mUsage;			// Usage of the texture

		/**
		 * Initializes a bitmap based on the settings associated with this texture
		 * @param bitmap the bitmap to initialize
		 */
		void initBitmap(opengl::Bitmap& bitmap);
		
	};
} // opengl
