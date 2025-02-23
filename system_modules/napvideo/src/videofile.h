/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <rtti/typeinfo.h>
#include <nap/resource.h>

namespace nap
{
	/**
	 * Path to a video file on disk. Can be loaded and played back by a nap::VideoPlayer.
	 * Verifies that the video exists and can be loaded on initialization.
	 */
	class NAPAPI VideoFile : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Checks if the file exists and can be loaded, also extracts the pixel format. Returns false if the file does not exist or cannot be loaded.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

        /**
         * @return the pixel format of the video file
         */
        int getPixelFormat() const { return mPixelFormat; }

		std::string mPath;			///< Property: 'Path' location of video file on disk
    private:
        const std::string avErrorToString(int err);

        int mPixelFormat = -1;       ///< Pixel format of the video file
	};
}
