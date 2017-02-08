// Local Includes
#include "nimage.h"
#include "nglutils.h"

namespace opengl
{	
	// Returns if the texture is compressed (on the GPU)
	bool Image::isCompressed()
	{
		if (!mTexture.isAllocated())
		{
			printMessage(MessageType::WARNING, "can't query image texture compression, no texture available");
			return false;
		}

		// Do an actual GPU lookup
		GLint size(0), type(0);
		return opengl::isCompressed(mTexture, size, type);
	}


	// Enables / Disables texture compression
	// If a texture is loaded, this call will update the texture on the GPU
	void Image::setCompressed(bool value)
	{
		// If we haven't loaded an image yet, simply set compression
		if (mLoaded)
		{
			// Change
			if (mCompress != value)
			{
				printMessage(MessageType::INFO, "set image: %s compression: %s", mPath.c_str(), value ? "true" : "false");
				setFromBitmap(mTexture, mBitmap, value);
			}

			// If compression was unsuccessful, revert
			if (value && !isCompressed())
			{
				printMessage(MessageType::WARNING, "image compression not supported: %s", mPath.c_str());
			}
		}
		mCompress = value;
	}

	// Load image defined with path
	bool Image::load()
	{
		// Clear bitmap
		mBitmap.clear();

		mLoaded = false;
		if (mPath.empty())
		{
			printMessage(MessageType::ERROR, "can't load image, no image path specified");
			return mLoaded;
		}

		// Load pixel data in to bitmap
		if (!loadBitmap(mBitmap, mPath))
		{
			printMessage(MessageType::ERROR, "unable to load image %s, invalid bitmap", mPath.c_str());
			return mLoaded;
		}

		// Make sure texture is allocated on GPU
		if (!mTexture.isAllocated())
			mTexture.init();

		// Upload texture data
		if (!setFromBitmap(mTexture, mBitmap, mCompress))
		{
			printMessage(MessageType::ERROR, "unable to load image %s, invalid hardware texture", mPath.c_str());
			return mLoaded;
		}

		// If image compression was turned on and compression was unsuccessful, turn it off
		if (mCompress && !isCompressed())
		{
			printMessage(MessageType::WARNING, "image compression not supported: %s", mPath.c_str());
		}

		mLoaded = true;
		return mLoaded;
	}


	// Binds the texture on the GPU
	void Image::bind() const
	{
		mTexture.bind();
	}

	
	// Unbinds the texture on the GPU
	void Image::unbind() const
	{
		mTexture.unbind();
	}


	// Returns the bitmap data
	const void* Image::getData() const
	{
		return mBitmap.getData();
	}


	// Loads an image from disk
	bool Image::load(const std::string& path)
	{
		// Set path
		mPath = path;
		return load();
	}


	// Reloads the image
	bool Image::reload()
	{
		return load();
	}


	// Returns image width
	unsigned int Image::getWidth() const
	{
		return mBitmap.getWidth();
	}


	// Returns image height
	unsigned int Image::getHeight() const
	{
		return mBitmap.getHeight();
	}

}