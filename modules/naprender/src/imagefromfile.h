#pragma once

// Local Includes
#include "texture2d.h"
#include "bitmap.h"

// External Includes
#include <rtti/rttiobject.h>
#include <utility/dllexport.h>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * An image holds both the cpu and gpu data associated
	 * with a 2d image, resulting in a 2d texture (GPU) and Bitmap (CPU)
	 * The bitmap is loaded automatically and populates the opengl texture
	 * with the right data on initialization
	 */
	class NAPAPI ImageFromFile : public Image
	{
		RTTI_ENABLE(Image)
	public:
		// Constructor
		ImageFromFile(const std::string& imgPath);

		// Default Constructor
		ImageFromFile() = default;

		/**
		* Loads the image from mImagePath.
		* @param errorState contains the error when initialization fails
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;


	public:
		// Path to img on disk
		std::string				mImagePath;								///< Property: 'ImagePath' Path to the image on disk to load
		bool					mCompressed = false;					///< Property: 'Compressed' If the image on the GPU is compressed
	};

}
