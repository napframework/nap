/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "datetime.h"

namespace nap
{
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
		void start();

		/**
		 * Returns the start time.
		 * @return timer start time
		 */
		std::chrono::time_point<Clock> getStartTime() const;

		/**
		 * Stops the timer, start time is set to 0.
		 * 
		 * This call is deprecated because setting the start time to 0 results in large time
		 * time deltas, depending on the clock that is used. This call therefore serves no purpose
		 * whatsoever and will be removed. 
		 */
		void stop();

		/**
		 * Resets the timer, essentially starting it again.
		 */
		void reset();

		/**
		* @return the elapsed time in seconds as double
		*/
		double getElapsedTime() const;

		/**
		* @return the elapsed time in seconds as a float
		*/
		float getElapsedTimeFloat() const;

	   /**
		* @return amount of processed ticks in milliseconds
		*/
		uint32_t getTicks() const;

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
		T get() const;

	private:
		// Members
		std::chrono::time_point<Clock> mStart;
	};


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
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename Clock>
	void Timer<Clock>::start()
	{
		mStart = Clock::now();
	}


	template<typename Clock>
	std::chrono::time_point<Clock> Timer<Clock>::getStartTime() const
	{
		return mStart;
	}


	// Stop timer
	template<typename Clock>
	void Timer<Clock>::stop()
	{
		mStart = std::chrono::time_point<Clock>(Milliseconds(0));
	}


	// Reset the timer
	template<typename Clock>
	void Timer<Clock>::reset()
	{
		start();
	}


	// Return number of ticks in milliseconds
	template<typename Clock>
	uint32_t Timer<Clock>::getTicks() const
	{
		return getMillis().count();
	}


	template<typename Clock>
	template<typename T>
	T nap::Timer<Clock>::get() const
	{
		return std::chrono::duration_cast<T>(Clock::now() - mStart);
	}


	// Return elapsed time in seconds
	template<typename Clock>
	double Timer<Clock>::getElapsedTime() const
	{
		return std::chrono::duration<double>(Clock::now() - mStart).count();
	}


	// Elapsed time in seconds
	template<typename Clock>
	float Timer<Clock>::getElapsedTimeFloat() const
	{
		return std::chrono::duration<float>(Clock::now() - mStart).count();
	}
}
