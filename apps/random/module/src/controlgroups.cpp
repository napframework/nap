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

	const nap::ControlGroups::ControlGroup& ControlGroups::getGroup(int index) const
	{
		assert(index < mControlGroups.size());
		return mControlGroups[index];
	}


	const nap::ControlGroups::ControlGroup* ControlGroups::findGroup(int controlIndex) const
	{
		// Find group for index
		const ControlGroup* found_group = nullptr;
		for (const auto& group : mControlGroups)
		{
			// Check if index is present in this group
			auto found_it = std::find_if(group.mControlGroupIndexes.begin(), group.mControlGroupIndexes.end(), [&](const auto& it)
			{
				return it == controlIndex;
			});

			// If so store handle to group
			if (found_it != group.mControlGroupIndexes.end())
				return &group;
		}
	}
}
