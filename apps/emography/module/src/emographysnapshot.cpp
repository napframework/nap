#include "emographysnapshot.h"

// nap::emographysnapshot run time class definition 
RTTI_BEGIN_STRUCT(nap::emography::Snapshot)
	RTTI_VALUE_CONSTRUCTOR(nap::emography::EStressState, float)
	RTTI_PROPERTY("State",		&nap::emography::Snapshot::mState,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Intensity",	&nap::emography::Snapshot::mIntensity,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TimeStamp",	&nap::emography::Snapshot::mTimeStamp,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{

		Snapshot::Snapshot(EStressState state, float intensity) : 
			mState(state), mIntensity(intensity), mTimeStamp(utility::getCurrentTime())
		{ }


		Snapshot::Snapshot(EStressState state, float intensity, utility::SystemTimeStamp stamp) :
			mState(state), mIntensity(intensity), mTimeStamp(stamp)
		{ }

	}
}