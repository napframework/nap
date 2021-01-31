/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "calendarcomponent.h"

// External Includes
#include <entity.h>
#include <rtti/rttiutilities.h>

// nap::calendarcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CalendarComponent)
	RTTI_PROPERTY("Frequency",	&nap::CalendarComponent::mFrequency,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Calendar",	&nap::CalendarComponent::mCalendar,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::calendarcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CalendarComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CalendarComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get instance
		CalendarComponent* resource = getComponent<CalendarComponent>();
		if (!errorState.check(resource->mCalendar != nullptr, "%s: No calendar", mID.c_str()))
			return false;
		mInstance = &resource->mCalendar->getInstance();
		
		// Get interval
		if (!errorState.check(resource->mFrequency > 0.0f, "%s: invalid update frequency, must be higher than 0", mID.c_str()))
			return false;
		mInterval = 1.0f / resource->mFrequency;

		// Listen to state changes
		mInstance->itemRemoved.connect(mItemRemoved);
		mInstance->itemAdded.connect(mItemAdded);

		return true;
	}


	void CalendarComponentInstance::update(double deltaTime)
	{
		// Skip if not dirty and not ready
		mTime += deltaTime;
		if (!mDirty && mTime < mInterval)
			return;

		// First handle previously deleted items.
		// These are no longer part of the data model (calendar) but could be in the active list. 
		// This ensures that listeners that received a (potential) 'eventStarted' trigger
		// get notified that that the item is no longer available and therefore ended.
		assert(mInstance != nullptr);
		SystemTimeStamp current_time = getCurrentTime();
		for (const auto& item : mDeletedItems)
		{
			auto it = mActive.find(item->mID);
			if (it != mActive.end())
			{
				eventEnded.trigger({ *item });
				mActive.erase(it);
			}
		}
		mDeletedItems.clear();

		// Now update all available calendar items
		for (const auto& item : mInstance->getItems())
		{
			// Based on active state of item and presence in list, take action.
			// If item is active but not present -> event started
			// If item is not active but present -> event finished
			if (item->active(current_time))
			{
				auto it = mActive.emplace(item->mID);
				if (it.second)
				{
					eventStarted.trigger({ *item });
				}
			}
			else
			{
				auto it = mActive.find(item->mID);
				if (it != mActive.end())
				{
					eventEnded.trigger({ *item });
					mActive.erase(it);
				}
			}
		}
		mTime  = 0.0;
		mDirty = false;
	}


	bool CalendarComponentInstance::active(const std::string& itemID) const
	{
		return mActive.find(itemID) != mActive.end();
	}


	void CalendarComponentInstance::onItemRemoved(const CalendarItem& item)
	{
		// Clone item, because we only notify listeners on update, 
		// to ensure app safety / validity.
		rtti::Factory& factory = getEntityInstance()->getCore()->getResourceManager()->getFactory();
		mDeletedItems.emplace_back(rtti::cloneObject(item, factory));
		mDirty = true;
	}
}