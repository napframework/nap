#pragma once

// Local Includes
#include "nbitmaputils.h"
#include "ntextureutils.h"

// External Includes
#include <string>
#include <memory>

namespace opengl
{
	/**
	 * Image
	 *
	 * An image combines a bitmap and texture
	 * The image path is used to construct a bitmap
	 * The constructed bitmap is uploaded to the GPU
	 *
	 * This is a convenience wrapper to easily load data from disk
	 * as a texture on to the GPU. Use the Bitmap and Texture classes
	 * for more fine grained control.
	 *
	 * The Image owns it's data, data is cleared on destruction
	 * Use the load call to load an image. 
	 *
	 * Always call load, constructing an Image with a path does not
	 * automatically load it!
	 */
	class Image
	{
	public:
		// Constructor
		Image() = default;
		Image(const std::string& path) : mPath(path)	{ }
		
		// Destructor
		virtual ~Image() = default;

		// Don't support copy
		// We can't copy GPU textures
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

		// Getters
		const Texture2D&		getTexture() const 				{ return mTexture; }
		const Bitmap&			getBitmap()	 const				{ return mBitmap; }
		
		// Width / Height
		unsigned int			getWidth()   const;
		unsigned int			getHeight()  const;	

		// Path
		const std::string&		getPath() const			{ return mPath; }

		/**
		 * isCompressed
		 *
		 * Checks if the image is compressed on the GPU
		 * This call performs an actual check
		 * if no image is loaded the result will always be false
		 */
		bool					isCompressed();

		/**
		 * setCompressed
		 *
		 * Turn GPU texture compression on / off
		 * If this image is loaded, changing this value will update the texture data on the GPU.
		 */
		void					setCompressed(bool value);

		/**
		 * bind
		 *
		 * binds the image on the GPU
		 * note that this call will fail if no image is loaded
		 */
		void					bind();
		
		/**
		 * unbind
		 *
		 * unbind the image on the GPU
		 * note that this call will fail if no image is loaded
		 */
		void					unbind();

		/**
		 * getData
		 *
		 * Returns the bitmap data
		 * @return: pointer to start of pixel data, nullptr if no image is loaded
		 */
		const void*				getData() const;

		/**
		 * isLoaded
		 *
		 * @return: if the image is loaded
		 * A loaded image has pixel data and a matching 2DTexture on the GPU 
		 */
		bool					isLoaded() const			{ return mLoaded; }

		/**
		 * load
		 *
		 * Loads the image in to memory
		 * Initializes the texture
		 * Generates for that bitmap an OpenGL Texture2D
		 * This function will always clear the bitmap data, also when failing to load the image
		 * @param path: the absolute path to the image on disk to load
		 * @return if load was successful or not
		 */
		bool					load(const std::string& path);
		
		/**
		 * load
		 *
		 * Loads the the image stored in path to memory
		 * Initializes the 2DTexture
		 * Uploads the bitmap to the 2DTexture on the GPU
		 * This function will always clear the bitmap data, also when failing to load the image
		 */
		bool					load();

		/**
		 * reload
		 *
		 * Reloads the current image
		 * Reloads the bitmap and refreshes the GPU texture
		 */
		bool					reload();

	private:
		Bitmap					mBitmap;				// Bitmap loaded from disk
		Texture2D				mTexture;				// OpenGL Texture
		std::string				mPath;					// Image Path
		bool					mLoaded		= false;	// If the bitmap and texture are loaded
		bool					mCompress	= false;	// If the image needs to be compressed or not
	};
}
