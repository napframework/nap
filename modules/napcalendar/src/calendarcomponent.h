/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "calendar.h"
#include "calendarevent.h"

// External Includes
#include <component.h>
#include <nap/timer.h>
#include <map>
#include <nap/signalslot.h>

namespace nap
{
	class CalendarComponentInstance;

	/**
	 * Notifies listeners when a calendar event starts and ends.
	 * Time used = local computer time. TODO: Add support for timezones.
	 */
	class NAPAPI CalendarComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CalendarComponent, CalendarComponentInstance)
	public:
		float mFrequency = 1.0f;								///< Property: 'Frequency' how many times a second the calendar is checked for changes. Defaults to 1x a second.
		nap::ResourcePtr<ICalendar> mCalendar = nullptr;		///< Property: 'Calendar' the calendar to watch
	};


	/**
	 * Notifies listeners when a calendar event starts and ends.
	 * Listen to the 'eventStarted' and 'eventEnded' signals to get notified of changes.
	 * This component also ensures that the eventEnded signal is called 
	 * when an item is removed or changed, if that item was previously in session.
	 * The state change checks are performed at a regular interval, 
	 * controlled by the CalendarComponent::Frequency property.
	 * Time used = local computer time. TODO: Add support for timezones.
	 */
	class NAPAPI CalendarComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CalendarComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize the component based on the resource
		 * @param errorState contains the error when initialization fails
		 * @return if the component initialized
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Checks for calendar item state changes if time passed or when dirty.
		 * Notifies listeners if an event started or ended.
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Forces a calendar activity check next update call.
		 * Call this after updating a calendar item.
		 */
		void setDirty()										{ mDirty = true; }

		/**
		 * @return the calendar this component monitors
		 */
		const nap::ICalendar& getCalendar() const			{ assert(mCalendar != nullptr);  return *mCalendar; }

		/**
		 * @return the calendar this component monitors
		 */
		nap::ICalendar& getCalendar()						{ assert(mCalendar != nullptr);  return *mCalendar; }

		/**
		 * Checks if a specific calendar item is currently active according to this component.
		 * Note that this check is often faster than: CalendarEvent::active(), because
		 * the active state is cached by this component, making it faster to query, especially when
		 * iterating and 'asking' a large number of calendar items.
		 *
		 * ~~~~~{.cpp}
		 *	for (const auto& item : calendar_component.getCalendar().getItems())
		 *	{
		 *		c_active = calendar_comp.active(item->mID);
		 *		...
		 *	}
		 * ~~~~~
		 *
		 * @param itemID the unique id of the calendar item
		 * @return if an item is currently active, according to this component.
		 */
		bool active(const std::string& itemID) const;

		nap::Signal<const CalendarEvent&>	eventStarted;	///< Triggered when a calendar event starts, always on the main thread on update()
		nap::Signal<const CalendarEvent&>	eventEnded;		///< Triggered when a calendar event ends, always on the main thread on update()

	private:
		nap::ICalendar* mCalendar = nullptr;
		float mInterval = 1.0f;
		double mTime = 0.0;
		std::unordered_set<std::string> mActive;			///< List of currently active calendar items by id
		OwnedCalendarItemList mDeletedItems;				///< List of deleted calendar items
		bool mDirty = true;

		/**
		 * Called when an item is added to the calendar.
		 * @param item the item that is added
		 */
		void onItemAdded(const CalendarItem& item)			{ mDirty = true; }
		nap::Slot<const CalendarItem&> mItemAdded =			{ this, &CalendarComponentInstance::onItemAdded };

		/**
		 * Called when an item is removed from the calendar.
		 * @param item the item to remove
		 */
		void onItemRemoved(const CalendarItem& item);
		nap::Slot<const CalendarItem&> mItemRemoved =		{ this, &CalendarComponentInstance::onItemRemoved };
	};
}