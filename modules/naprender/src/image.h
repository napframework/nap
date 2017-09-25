#pragma once

// Local Includes
#include "texture.h"

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Wraps an opengl image 
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and bitmap data (CPU)
	 */
	class NAPAPI Image : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
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

	public:
		// Path to img on disk
		std::string				mImagePath;
		bool					mStoreCompressed = false;
	};

}
