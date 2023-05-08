/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "image.h"

// External Includes
#include <rtti/factory.h>

namespace nap
{
	/**
	 * Contains both the CPU and GPU data of a 2D image that is loaded from disk.
	 * The bitmap is loaded on initialization and scheduled for upload, into a texture on the GPU, on success.
	 * Mip-maps are automatically generated when 'GenerateLods' is set to true.
	 */
	class NAPAPI ImageFromFile : public Image
	{
		RTTI_ENABLE(Image)
	public:
		/**
		 * @param core the core instance.
		 * @param imgPath path to the image on disk.
		 */
		ImageFromFile(Core& core, const std::string& imgPath);

		/**
		 * @param core the core instance
		 */
		ImageFromFile(Core& core);

		/**
		* Loads the image from disk and schedules the upload to the GPU on success.
		* @param errorState contains the error when initialization fails
		* @return true when successful, otherwise false.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		std::string				mImagePath;								///< Property: 'ImagePath' Path to the image on disk to load
		bool					mGenerateLods = true;					///< Property: 'GenerateLods' If LODs are generated for this image
	};
}
