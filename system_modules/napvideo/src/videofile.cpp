/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "videofile.h"

// External Includes
#include <utility/fileutils.h>

RTTI_BEGIN_CLASS(nap::VideoFile, "Video loaded from disk")
	RTTI_PROPERTY_FILELINK("Path",	&nap::VideoFile::mPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Video, "Path to the video to load")
RTTI_END_CLASS

namespace nap
{
	bool VideoFile::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(utility::fileExists(mPath), "%s: file: %s does not exist", mID.c_str(), mPath.c_str()))
			return false;
		return true;
	}
}
