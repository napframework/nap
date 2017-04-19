#pragma once

#include <nap.h>
#include <nap/resource.h>
#include <nap/coreattributes.h>
#include <nimage.h>

namespace nap
{
	class ImageResourceLoader;

	/**
	 * Base class for texture resources
	 */
	class TextureResource : public Resource
	{
		friend class ImageResourceLoader;
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		/**
		 * Virtual override to be implemented by derived classes
		 */
		virtual const opengl::BaseTexture& getTexture() const = 0;

		/**
		 * Non const accessors
		 */
		opengl::BaseTexture& getTexture();

		/**
		 * Binds the texture
		 */
		virtual bool bind()					{ return getTexture().bind(); }

		/**
		 * Unbinds the texture
		 */
		virtual bool unbind() 				{ return getTexture().unbind(); }
	};

	/**
	* 2D Texture resource that only has an in-memory representation.
	*/
	class MemoryTextureResource2D : public TextureResource
	{
		RTTI_ENABLE_DERIVED_FROM(TextureResource)
	public:

		/**
		* Initializes 2D texture. Additionally a custom display name can be provided.
		*/
		virtual bool init(InitResult& initResult) override;

		/**
		* Returns 2D texture object
		*/
		virtual const opengl::BaseTexture& getTexture() const override;

		/**
		* Returns custom display name
		*/
		virtual const std::string getDisplayName() const override				{ return mDisplayName;  }

	public:
		//opengl::Texture2DSettings mSettings;
		NumericAttribute<int>	mLevel			= { this, "mLevel", 0 };
		NumericAttribute<int>	mInternalFormat	= { this, "mInternalFormat", GL_RGB };
		NumericAttribute<int>	mWidth			= { this, "mWidth", 0 };
		NumericAttribute<int>	mHeight			= { this, "mHeight", 0 };
		NumericAttribute<int>	mFormat			= { this, "mFormat", GL_BGR };
		NumericAttribute<int>	mType			= { this, "mType", GL_UNSIGNED_BYTE };

	private:
		opengl::Texture2D mTexture;						// Texture as created during init
		std::string mDisplayName = "MemoryTexture2D";	// Custom display name
	};


	/**
	 * Wraps an opengl image 
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and bitmap data (CPU)
	 */
	class ImageResource : public TextureResource
	{
		friend class ImageResourceLoader;
		RTTI_ENABLE_DERIVED_FROM(TextureResource)
	public:
		// Constructor
		ImageResource(const std::string& imgPath);

		// Default Constructor
		ImageResource() = default;

		virtual bool init(InitResult& initResult) override;

		/**
		 * @return opengl image + bitmap data
		 * Note that this implicitly loads the image
		 * Make sure that the image is loaded successfully
		 */
		const opengl::Image& getImage() const;

		/**
		 * @return opengl texture object
		 * Note that this implicitly loads the image
		 */
		virtual const opengl::BaseTexture& getTexture() const override;

		/**
		 * @return human readable display name
		 */
		virtual const std::string getDisplayName() const override;

	public:
		// Path to img on disk
		Attribute<std::string>	mImagePath = { this, "mImagePath", "" };

	private:
		// Display name of img
		std::string				mDisplayName;

		// Opengl Image Object
		mutable opengl::Image	mImage;
	};

}

RTTI_DECLARE_BASE(nap::TextureResource)
RTTI_DECLARE(nap::MemoryTextureResource2D)
RTTI_DECLARE(nap::ImageResource)
