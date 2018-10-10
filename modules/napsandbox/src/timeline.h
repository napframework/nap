#pragma once

#include <nap/resource.h>
#include <component.h>

namespace nap
{

	class TimelineEvent
	{
	public:
		/**
		 * Invoked when the event is hit for the first time. Use this to set up necessary initialization.
		 * Also, update(0) will be called once.
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

		double startTime() const { return mStartTime; }
		void setStartTime(double start) { mStartTime = start; }
		double endTime() const { return mEndTime; }
		void setEndTime(double end) { mEndTime = end; }

	private:
		double mStartTime = 0;
		double mEndTime = 1;

	};


	class TimelineComponentInstance;

	class TimelineComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TimelineComponent, TimelineComponentInstance)
	public:

	};

	class TimelineComponentInstance : public ComponentInstance
	{
	public:
		double currentTime() const { return mTime; }
		float speed() const { return mSpeed; }
		void setSpeed(float speed) { mSpeed = speed; }

		void update(double deltaTime) override
		{
			mTime += deltaTime * mSpeed;
		}
	private:
		double mTime;
		float mSpeed = 1.0;
	};

}