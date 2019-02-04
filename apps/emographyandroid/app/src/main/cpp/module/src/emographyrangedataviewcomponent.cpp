#include "emographyrangedataviewcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/core.h>

RTTI_BEGIN_STRUCT(nap::emography::RangeDataSettings)
	RTTI_PROPERTY("Samples",	&nap::emography::RangeDataSettings::mSamples,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StartTime",	&nap::emography::RangeDataSettings::mStartTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EndTime",	&nap::emography::RangeDataSettings::mEndTime,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::emographydataviewcomponent run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::RangeDataViewComponent)
	RTTI_PROPERTY("Settings", &nap::emography::RangeDataViewComponent::mSettings,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DataModel", &nap::emography::RangeDataViewComponent::mDataModel, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::emographydataviewcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::RangeDataviewComponentInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool emography::RangeDataviewComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch API Service
		mAPIService = getEntityInstance()->getCore()->getService<nap::APIService>();
		if (!errorState.check(mAPIService != nullptr, "%s: no api service found", mID.c_str()))
			return false;

		// Get pointer to original resource
		RangeDataViewComponent* resource = getComponent<RangeDataViewComponent>();

		// Copy settings
		RangeDataSettings& settings = resource->mSettings;
		mSampleCount = math::max<int>(1, settings.mSamples);
		mStartTime = TimeStamp(settings.mStartTime.toSystemTime());
		mEndTime = TimeStamp(settings.mEndTime.toSystemTime());

		// Acquire handle to data model, we know (now) that it has been initialized correctly.
		mDataModel = &(resource->mDataModel->getInstance());
		return true;
	}


	void emography::RangeDataviewComponentInstance::query(const TimeStamp& startTime, const TimeStamp& endTime, int samples)
	{
		mStartTime = startTime;
		mEndTime = endTime;
		mSampleCount = math::max<int>(samples, 1);
		onQuery();
	}
}