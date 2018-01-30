#pragma once

// Local Includes
#include "texture2d.h"
#include "pixmap.h"

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
	class NAPAPI Texture2DFromFile : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		// Constructor
		Texture2DFromFile(const std::string& imgPath);

		// Default Constructor
		Texture2DFromFile() = default;

		/**
		* Loads the image from mImagePath.
		* @param errorState contains the error when initialization fails
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;


	public:
		// Path to img on disk
		std::string				mImagePath;								///< Path to the image on disk to load
		bool					mCompressed = false;					///< If the image on the GPU is compressed
	};

}
