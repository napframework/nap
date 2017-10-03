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
	 * Wraps an opengl image 
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and bitmap data (CPU)
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
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the bitmap associated with this image
		 */
		const opengl::Bitmap& getBitmap() const							{ return mBitmap; }

	public:
		// Path to img on disk
		std::string				mImagePath;
		bool					mCompressed = false;
		opengl::Bitmap			mBitmap;
	};

}
