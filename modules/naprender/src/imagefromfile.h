#pragma once

// Local Includes
#include "image.h"

// External Includes
#include <rtti/factory.h>

namespace nap
{
	/**
	 * An image from file holds both the cpu and gpu data associated with a 2d image, 
	 * resulting in a 2d texture (GPU) and Bitmap (CPU)
	 * The bitmap is loaded automatically and populates the opengl texture with the right data on initialization
	 */
	class NAPAPI ImageFromFile : public Image
	{
		RTTI_ENABLE(Image)
	public:
		// Constructor using image path
		ImageFromFile(Core& core, const std::string& imgPath);

		// Default Constructor
		ImageFromFile(Core& core);

		/**
		* Loads the image from mImagePath.
		* @param errorState contains the error when initialization fails
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		std::string				mImagePath;								///< Property: 'ImagePath' Path to the image on disk to load
		bool					mGenerateLods = true;					///< Property: 'GenerateLods' If LODs are generated for this image
	};
}
