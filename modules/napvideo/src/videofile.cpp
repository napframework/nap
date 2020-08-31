// Local Includes
#include "videofile.h"

// External Includes
#include <utility/fileutils.h>

RTTI_BEGIN_CLASS(nap::VideoFile)
	RTTI_PROPERTY_FILELINK("Path",	&nap::VideoFile::mPath,		nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Video)
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