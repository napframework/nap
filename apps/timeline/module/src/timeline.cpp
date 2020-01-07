// local includes
#include "timeline.h"

RTTI_BEGIN_CLASS(nap::Timeline)
RTTI_PROPERTY("Name", &nap::Timeline::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Parameters", &nap::Timeline::mParameters, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("KeyFrames", &nap::Timeline::mKeyFrames, nap::rtti::EPropertyMetaData::Embedded)
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
