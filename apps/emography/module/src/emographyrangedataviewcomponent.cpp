#include "emographyrangedataviewcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

RTTI_BEGIN_STRUCT(nap::emography::RangeDataSettings)
	RTTI_PROPERTY("Samples",	&nap::emography::RangeDataSettings::mSamples,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("StartTime",	&nap::emography::RangeDataSettings::mStartTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EndTime",	&nap::emography::RangeDataSettings::mEndTime,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::emographydataviewcomponent run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::RangeDataViewComponent)
	RTTI_PROPERTY("Settings", &nap::emography::RangeDataViewComponent::mSettings,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::emographydataviewcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::RangeDataviewComponentInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool emography::RangeDataviewComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy data
		RangeDataSettings& settings = getComponent<RangeDataViewComponent>()->mSettings;
		mSampleCount = math::max<int>(1, settings.mSamples);
		setTimeRange(settings.mStartTime.toSystemTime(), settings.mEndTime.toSystemTime());
		return true;
	}


	void emography::RangeDataviewComponentInstance::setSampleCount(int count)
	{
		mSampleCount = math::max<int>(1, count);
		settingsChanged();
	}


	void emography::RangeDataviewComponentInstance::setTimeRange(const utility::SystemTimeStamp& begin, const utility::SystemTimeStamp& end)
	{
		mStartTime = begin;
		mEndTime = end;
		settingsChanged();
	}
}