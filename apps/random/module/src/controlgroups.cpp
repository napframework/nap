// Local Includes
#include "controlgroups.h"

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_STRUCT(nap::ControlGroups::ControlGroup)
RTTI_PROPERTY("Name", &nap::ControlGroups::ControlGroup::mName, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Brightness", &nap::ControlGroups::ControlGroup::mBrightness, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("ControlGroupIndexes", &nap::ControlGroups::ControlGroup::mControlGroupIndexes, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::ControlGroups)
RTTI_PROPERTY("ControlGroups", &nap::ControlGroups::mControlGroups, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool ControlGroups::init(utility::ErrorState& errorState)
	{
		return true;
	}
}
