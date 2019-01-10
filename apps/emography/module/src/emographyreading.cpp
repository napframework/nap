#include "emographyreading.h"

// nap::emography::reading run time class definition 
RTTI_BEGIN_CLASS(nap::emography::Reading)
	RTTI_PROPERTY("Snapshots",	&nap::emography::Reading::mSnapshots,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StartTime",	&nap::emography::Reading::mStartTime,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EndTime",	&nap::emography::Reading::mEndTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",	&nap::emography::Reading::mSamples,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		Reading::~Reading() { }


		bool Reading::init(utility::ErrorState& errorState)
		{
			return true;
		}
	}
}