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
		 * Checks for calendar item state changes.
		 * Notifies listeners if an event started or ended.
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the calendar this component monitors
		 */
		const nap::CalendarInstance& getCalendar() const	{ assert(mInstance != nullptr);  return *mInstance; }

		/**
		 * @return the calendar this component monitors
		 */
		nap::CalendarInstance& getCalendar()				{ assert(mInstance != nullptr);  return *mInstance; }

		/**
		 * Checks if a specific calendar item is currently active according to this component.
		 * Note that this check is often faster than: CalendarEvent::active(), because
		 * the active state is cached by this component, making it faster to query, especially when
		 * iterating and 'asking' a large number of calendar items.
		 * 
		 * @param itemID the unique id of the calendar item
		 * @return if an item is currently active, according to this component.
		 */
		bool active(const std::string& itemID) const;

		nap::Signal<const CalendarEvent&>	eventStarted;	///< Triggered when a calendar event starts, always on the main thread on update()
		nap::Signal<const CalendarEvent&>	eventEnded;		///< Triggered when a calendar event ends, always on the main thread on update()

	private:
		nap::CalendarInstance* mInstance = nullptr;
		float mInterval = 1.0f;
		double mTime = 0.0;
		std::unordered_set<std::string> mActive;			///< List of currently active calendar items by id
		OwnedCalendarItemList mDeletedItems;				///< List of deleted calendar items

		/**
		 * Called when an item is removed from the calendar.
		 * Calls 'eventEnded' if the item is currently active.
		 * @param item the item to remove
		 */
		void onItemRemoved(const CalendarItem& item);
		nap::Slot<const CalendarItem&> mItemRemoved =		{ this, &CalendarComponentInstance::onItemRemoved };
	};
}
