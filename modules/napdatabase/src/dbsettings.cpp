#include "dbsettings.h"

// nap::dbsettings run time class definition 
RTTI_BEGIN_CLASS(nap::DBSettings)
	RTTI_PROPERTY("Settings", &nap::DBSettings::mSettings, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::dbsetting run time class definition 
RTTI_BEGIN_STRUCT(nap::DBSetting)
	RTTI_VALUE_CONSTRUCTOR(std::string, std::string)
	RTTI_PROPERTY("Key",	&nap::DBSetting::mKey,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Value",	&nap::DBSetting::mValue,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	DBSetting::DBSetting(std::string key, std::string value) : mKey(key), mValue(value)
	{

	}


	bool DBSettings::init(utility::ErrorState& errorState)
	{
		int index = 0;
		for (const auto& kv : mSettings)
		{
			if (!errorState.check(!kv.mKey.empty(), "%s: contains an invalid key at index: %s", mID.c_str(), index))
				return false;
			index++;
		}
		return true;
	}
}