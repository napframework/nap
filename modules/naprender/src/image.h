#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <nap/dllexport.h>
#include <nimage.h>
#include <glm/glm.hpp>

namespace nap
{
	class ImageResourceLoader;

	/**
	 * Base class for texture resources
	 */
	class NAPAPI Texture : public rtti::RTTIObject
	{
		friend class ImageResourceLoader;
		RTTI_ENABLE(rtti::RTTIObject)
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
		 * Virtual override to get the size of the texture, to be implemented by derived classes
		 */
		virtual const glm::vec2 getSize() const = 0;

		/**
		 * Binds the texture
		 */
		virtual bool bind()					{ return getTexture().bind(); }

		/**
		 * Unbinds the texture
		 */
		virtual bool unbind() 				{ return getTexture().unbind(); }

		/**
		 *	Holds all the texture related parameters
		 */
		opengl::TextureParameters			mParameters;
	};

	/**
	* 2D Texture resource that only has an in-memory representation.
	*/
	class NAPAPI MemoryTexture2D : public Texture
	{
		RTTI_ENABLE(Texture)
	public:

		/**
		* Creates internal texture resource.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Returns 2D texture object
		*/
		virtual const opengl::BaseTexture& getTexture() const override;

		/**
		 * Get the size of the texture
		 */
		virtual const glm::vec2 getSize() const override;

	public:
		opengl::Texture2DSettings mSettings;

	private:
		std::unique_ptr<opengl::Texture2D> mTexture;				// Texture as created during init
		std::string mDisplayName = "MemoryTexture2D";				// Custom display name
	};


	/**
	 * Wraps an opengl image 
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and bitmap data (CPU)
	 */
	class NAPAPI Image : public Texture
	{
		friend class ImageResourceLoader;
		RTTI_ENABLE(Texture)
	public:
		// Constructor
		Image(const std::string& imgPath);

		// Default Constructor
		Image() = default;

		/**
		* Loads the image from mImagePath.
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

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
		 * Get the size of the texture
		 */
		virtual const glm::vec2 getSize() const override;

	public:
		// Path to img on disk
		std::string				mImagePath;

	private:
		// Opengl Image Object
		std::unique_ptr<opengl::Image>	mImage = nullptr;
	};

}
