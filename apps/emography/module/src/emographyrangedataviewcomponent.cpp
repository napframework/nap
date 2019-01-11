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
		setSettings(getComponent<RangeDataViewComponent>()->mSettings);
		return true;
	}


	void emography::RangeDataviewComponentInstance::setSettings(const RangeDataSettings& settings)
	{
		mSettings = settings;
		mSettings.mSamples = math::max<int>(1, settings.mSamples);
		settingsChanged();
	}
}