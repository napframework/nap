#include "calendarcomponent.h"

// External Includes
#include <entity.h>

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
		if (!errorState.check(resource->mCalendar == nullptr, "%s: No calendar", mID.c_str()))
			return false;
		mInstance = &resource->mCalendar->getInstance();
		
		// Get interval
		if (!errorState.check(resource->mFrequency > 0.0f, "%s: invalid update frequency, must be higher than 0", mID.c_str()))
			return false;
		mInterval = 1.0f / resource->mFrequency;

		// Setting time to interval ensures that calendar is checked first cycle
		mTime = mInterval;

		return true;
	}


	void CalendarComponentInstance::update(double deltaTime)
	{
		// Skip if check isn't required
		mTime += deltaTime;
		if (mTime < mInterval)
			return;

		assert(mInstance != nullptr);
		SystemTimeStamp current_time = getCurrentTime();
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
		mTime = 0.0;
	}


	void CalendarComponentInstance::onItemRemoved(const CalendarItem& item)
	{
		auto it = mActive.find(item.mID);
		if (it != mActive.end())
		{
			eventEnded.trigger({ item });
			mActive.erase(it);
		}
	}
}