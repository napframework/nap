#pragma once

#include <nap/resource.h>
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
		virtual std::string getDisplayName() const override;


	private:
		// Path to img on disk
		std::string				mImagePath;

		// Display name of img
		std::string				mDisplayName;

		// If the img has been loaded
		mutable bool			mLoaded = false;

		// Opengl Image Object
		mutable opengl::Image	mImage;
	};


	/**
	 * Creates the image resource
	 * for a list of supported formats check: http://freeimage.sourceforge.net/features.html
	 */
	class ImageResourceLoader : public ResourceLoader
	{
		RTTI_ENABLE_DERIVED_FROM(ResourceLoader)
	public:
		ImageResourceLoader();

		/**
		 * @return all supported image extensions
		 */
		static const std::vector<std::string>& getSupportedImgExtensions();

		/**
		 * Creates an image resource
		 */
		virtual std::unique_ptr<Resource> loadResource(const std::string& resourcePath) const override;
	};
}

RTTI_DECLARE_BASE(nap::TextureResource)
RTTI_DECLARE(nap::ImageResource)
RTTI_DECLARE(nap::ImageResourceLoader)