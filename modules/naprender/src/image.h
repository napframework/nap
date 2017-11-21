#pragma once

// Local Includes
#include "basetexture2d.h"

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>
#include <nbitmap.h>

namespace nap
{
	/**
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and Bitmap (CPU)
	 * The bitmap is loaded automatically and populates the opengl texture
	 * with the right data on initialization
	 */
	class NAPAPI Image : public BaseTexture2D
	{
		RTTI_ENABLE(BaseTexture2D)
	public:
		// Constructor
		Image(const std::string& imgPath);

		// Default Constructor
		Image() = default;

		/**
		* Loads the image from mImagePath.
		* @param errorState contains the error when initialization fails
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the bitmap associated with this image
		 */
		const opengl::Bitmap& getBitmap() const							{ return mBitmap; }

	public:
		// Path to img on disk
		std::string				mImagePath;								///< Path to the image on disk to load
		bool					mCompressed = false;					///< If the image on the GPU is compressed
		opengl::Bitmap			mBitmap;								///< The CPU image representation
	};

}
