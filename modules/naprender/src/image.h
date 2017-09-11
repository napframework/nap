#pragma once

// Local Includes
#include "texture.h"

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <nimage.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Wraps an opengl image 
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and bitmap data (CPU)
	 */
	class NAPAPI Image : public Texture
	{
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
