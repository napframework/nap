#pragma once

#include "ntexture.h"

namespace opengl
{
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

		void init(/*const Texture2DSettings& textureSettings,*/ const TextureParameters& parameters, ETextureUsage usage);

		/**
		 * @return Texture2D settings object
		 */
		//const Texture2DSettings& getSettings() const { return mSettings; }

		/**
		 * setData
		 * 
		 * Uploads 2d pixel data to the GPU
		 * Make sure that the data the pointer points at matches the size of the texture settings provided!
		 *
		 * @param data Pointer to the appropriately-sized buffer to copy to the texture
		 * @param pitch The size (in bytes) of each row of pixels. The default of 0 means that it will use the internal opengl format to determine this.
		 */
		void setData(const void* data, int pitch = 0);

		/**
		 * @return The size of the texture when copied to/from CPU.
		 */
		int getDataSize() const;

		/**
		 * Blocking call to retrieve GPU texture data. 
		 * @param target Block of data that is filled with texture data
		 * @param sizeInBytes The size of the target buffer in bytes. Must be greater than or equal to the size of the texture (see getDataSize())
		 */
		void getData(void* target, uint64_t sizeInBytes);

		/**
		 * Starts a transfer of texture data from GPU to CPU. Use asyncEndGetData to block waiting for the async command to complete.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered. It is recommended
		 * to use double or triple buffering to make sure that no stalls occur when calling asyncEndGetData().
		 */
		void asyncStartGetData();

		/**
		 * Finishes a transfer of texture data from GPU to CPU that was started with asyncStartGetData. See comment in asyncStartGetData for proper use.
		 * @param target Block of data that is filled with texture data
		 * @param sizeInBytes The size of the target buffer in bytes. Must be greater than or equal to the size of the texture (see getDataSize())
		 */
		void asyncEndGetData(void* target, uint64_t sizeInBytes);

	private:
		//Texture2DSettings	mSettings;		// Settings object
		GLuint				mPBO;			// Pixel buffer object used to read/write texture data if usage is DynamicRead or DynamicWrite
		ETextureUsage		mUsage;			// Usage of the texture
	};
} // opengl
