/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "operationalcalendar.h"

// nap::operationalcalendar run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OperationalCalendar)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("AllowLoadFailure",	&nap::OperationalCalendar::mAllowFailure,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Monday",				&nap::OperationalCalendar::mMonday,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Tuesday",			&nap::OperationalCalendar::mTuesday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Wednesday",			&nap::OperationalCalendar::mWednesday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Thursday",			&nap::OperationalCalendar::mThursday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Friday",				&nap::OperationalCalendar::mFriday,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Saturday",			&nap::OperationalCalendar::mSaturday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sunday",				&nap::OperationalCalendar::mSunday,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	OperationalCalendar::OperationalCalendar(nap::Core& core) : ICalendar(core) { }


	OperationalCalendar::~OperationalCalendar() 
	{
		mInstance.reset(nullptr);
	}


	bool OperationalCalendar::init(utility::ErrorState& errorState)
	{
		// Create set of items to initialize instance with
		CalendarItemList list;
		list.reserve(7);
		addItem(mMonday,	EDay::Monday,		list);
		addItem(mTuesday,	EDay::Tuesday,		list);
		addItem(mWednesday,	EDay::Wednesday,	list);
		addItem(mThursday,	EDay::Thursday,		list);
		addItem(mFriday,	EDay::Friday,		list);
		addItem(mSaturday,	EDay::Saturday,		list);
		addItem(mSunday,	EDay::Sunday,		list);

		// Create and initialize instance
		mInstance = std::make_unique<CalendarInstance>(mCore);
		if (!mInstance->init(mID.c_str(), mAllowFailure, list, errorState))
			return false;

		return true;
	}


	bool OperationalCalendar::isOperational()
	{
		assert(mInstance != nullptr);
		SystemTimeStamp current_time = getCurrentTime();
		for (auto& item : mInstance->getItems())
		{
			if (item->active(current_time))
				return true;
		}
		return false;
	}


	void OperationalCalendar::addItem(const CalendarItem::Point& point, EDay day, CalendarItemList& outItems)
	{
		auto item = mCore.getResourceManager()->createObject<WeeklyCalendarItem>();
		item->mPoint = point;
		item->mTitle = toString(day);
		item->mDay = day;
		outItems.emplace_back(item);
	}
}
