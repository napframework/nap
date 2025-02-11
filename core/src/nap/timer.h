/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "datetime.h"

namespace nap
{
	// Forward declares
	template<typename Clock>
	class Timer;

	//////////////////////////////////////////////////////////////////////////
	// Timers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Keeps track of time from the moment the timer is started.
	 * This timer uses the chrono SystemClock and should be sufficient for most time based operations.
	 * The timestamp associated with a SystemTimer can be converted to days, seconds, weeks etc.
	 */
	using SystemTimer = Timer<SystemClock>;


	/**
	 * Keeps track of time from the moment the timer is started.
	 * This timer uses the chrono HighResolutionClock and should be used when extreme accuracy is important.
	 * The timestamp associated with a HighResolutionTime can not be converted to days, seconds, weeks etc.
	 */
	using HighResolutionTimer = Timer<HighResolutionClock>;


	/**
	 * Keeps track of time from the moment the timer is started.
	 * This timer uses the chrono SteadyClock.
	 * This clock cannot move back in time and the time in between ticks is guaranteed to be constant.
	 */
	using SteadyTimer = Timer<SteadyClock>;


	//////////////////////////////////////////////////////////////////////////
	// Implementation
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Keeps track of time from the moment the timer is started.
	 * This is a template Timer that can work with various chrono clocks.
	 * Use the utility classes nap::SystemTimer and nap::HighResolutionTimer to work with specific clocks.
	 * The template type T should be a specific type of chrono clock, ie: HighResolutionClock etc.
	 * This timer is not threaded and doesn't work with callbacks.
	 */
	template<typename Clock>
	class Timer
	{
	public:
		// Construction / Destruction
		Timer() = default;
		virtual ~Timer() = default;

		/**
		 * Start the timer
		 */
		void start()												{ mStart = Clock::now(); }

		/**
		 * Start timer at given time
		 * @param time the start time
		 */
		void start(std::chrono::time_point<Clock> time)				{ mStart = time; }

		/**
		 * Returns the start time.
		 * @return timer start time
		 */
		std::chrono::time_point<Clock> getStartTime() const			{ return mStart; }

		/**
		 * Resets the timer, essentially starting it again.
		 */
		void reset()												{ start(); }

		/**
		* @return the elapsed time in seconds as double
		*/
		double getElapsedTime() const								{ return std::chrono::duration<double>(Clock::now() - mStart).count(); }

		/**
		* @return the elapsed time in seconds as a float
		*/
		float getElapsedTimeFloat() const							{ return std::chrono::duration<float>(Clock::now() - mStart).count(); }

	   /**
		* @return amount of processed ticks in milliseconds
		*/
		uint32_t getTicks() const									{ return getMillis().count(); }

	   /**
		* @return elapsed time in nanoseconds
		*/
		NanoSeconds getNanos() const								{ return get<NanoSeconds>(); }

	   /**
		* @return elapsed time in microseconds
		*/
		MicroSeconds getMicros() const								{ return get<MicroSeconds>(); }

	   /**
		* @return elapsed time in milliseconds
		*/
		Milliseconds getMillis() const								{ return get<Milliseconds>(); }

		/**
		 * @return elapsed time in seconds
		 */
		Seconds getSeconds() const									{ return get<Seconds>(); }

		/**
		 * @return elapsed time in minutes
		 */
		Minutes getMinutes() const									{ return get<Minutes>(); }

		/**
		 * @return elapsed time in hours
		 */
		Hours getHours() const										{ return get<Hours>(); }

		/**
		 * Utility function that casts this timer's duration to a duration of type T.
		 * Where T can be NanoSeconds, MicroSeconds, MilliSeconds etc.
		 * @return elapsed time as duration of type T (Microseconds, Milliseconds, Seconds, etc.)
		 */
		template<typename T>
		T get() const												{ return std::chrono::duration_cast<T>(Clock::now() - mStart); }

	private:
		// Members
		std::chrono::time_point<Clock> mStart;
	};
}
