// local includes
#include "timeline.h"

RTTI_BEGIN_CLASS(nap::Timeline)
RTTI_PROPERTY("Name", &nap::Timeline::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Tracks", &nap::Timeline::mTracks, nap::rtti::EPropertyMetaData::Embedded)
RTTI_PROPERTY("Duration", &nap::Timeline::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool Timeline::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		return true;
	}
}
