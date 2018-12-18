#pragma once

#include <nap/resource.h>
#include <component.h>

/**
 * A basic reference implementation of the timeline system
 */

namespace nap
{
	class TimelineEventInstance;

	/**
	 * A single event in a timeline, implementors may choose what to do during the time this event is active.
	 *
	 * Because there is a chance any event will be skipped over due to the speed of the timeline and how short an event is,
	 * the following methods will be called so the client code may ensure certain things happen when the event happens:
	 *
	 * start() will be called the first time this event becomes active.
	 * end() will be called the last time before the event is disposed of.
	 *
	 * In between, the timeline will continuously call update(double) at a set interval.
	 *
	 */
	class TimelineEventResource : public Resource
	{
	public:
		double mStartTime = 0;
		double mEndTime = 1;
		virtual std::unique_ptr<TimelineEventInstance> create() = 0;
	};

	/**
	 * Run-time instance of an event resource
	 */
	class TimelineEventInstance
	{
	public:
		TimelineEventInstance(TimelineEventResource& resource)  : mResource(resource) {}
		/**
		 * Invoked when the event is hit for the first time. Use this to set up necessary initialization.
		 * Also, update(0) will be called once to ensure proper update of any client code.
		 */
		virtual void start() = 0;

		/**
		 * Invoked when the event ends. This is the last call. After this, the event will be thrown away.
		 */
		virtual void end() = 0;

		/**
		 * This function will be called at the owning Timeline's tick rate. Implementors may keep a local time.
		 * @param deltaTime
		 */
		virtual void update(double deltaTime) = 0;

		const TimelineEventResource* getResource() const { return &mResource; }

	protected:
		TimelineEventResource& mResource;
	};

	class TimelineComponentInstance;

	/**
	 * A component representing a single timeline, holding a series of events.
	 */
	class TimelineComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TimelineComponent, TimelineComponentInstance)
	public:
		void addEvent(std::unique_ptr<TimelineEventResource> event) { mEvents.emplace_back(std::move(event)); }
		const std::vector<std::unique_ptr<TimelineEventResource>>& events() const { return mEvents; }
	private:
		std::vector<std::unique_ptr<TimelineEventResource>> mEvents;
	};

	/**
	 * Run-time instance of a TimelineComponent,
	 * will keep a local time and advance any active events based on their start/end times.
	 */
	class TimelineComponentInstance : public ComponentInstance
	{
	public:
		double currentTime() const { return mTime; }
		/**
		 * @return The speed multiplier that will be used to determine how fast this timeline progresses
		 */
		float speed() const { return mSpeed; }
		/**
		 * Set
		 * @param speed
		 */
		void setSpeed(float speed) { mSpeed = speed; }

		void update(double deltaTime) override;
		bool init(utility::ErrorState& errorState) override;

	private:
		void startNewEvents();
		void updateActiveEvents(double deltaTime);
		void endOldEvents();

		TimelineComponent* mTimelineComponent;
		std::vector<std::unique_ptr<TimelineEventInstance>> mQueuedEvents;
		std::vector<std::unique_ptr<TimelineEventInstance>> mActiveEvents;
//		std::vector<std::unique_ptr<TimelineEventInstance>> mFinishedEvents;
		double mTime;
		float mSpeed = 1.0;
	};

}