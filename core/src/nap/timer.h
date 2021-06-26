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
		* @return start time as point in time
		*/
		std::chrono::time_point<Clock> getStartTime() const;

		/**
		* Stop the timer, resets state
		*/
		void stop();

		/**
		* Resets the timer and starts it again
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
		* @return elapsed time in milliseconds
		*/
		Milliseconds getMillis();

		/**
		* @return elapsed time in microseconds
		*/
		MicroSeconds getMicros();

		/**
		*	@return elapsed time in nanoseconds
		*/
		NanoSeconds getNanos();

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
		auto elapsed = Clock::now() - mStart;
		return std::chrono::duration_cast<Milliseconds>(elapsed).count();
	}


	template<typename Clock>
	Milliseconds nap::Timer<Clock>::getMillis()
	{
		auto elapsed = Clock::now() - mStart;
		return std::chrono::duration_cast<Milliseconds>(elapsed);
	}


	template<typename Clock>
	NanoSeconds nap::Timer<Clock>::getNanos()
	{
		auto elapsed = Clock::now() - mStart;
		return std::chrono::duration_cast<NanoSeconds>(elapsed);
	}


	template<typename Clock>
	MicroSeconds nap::Timer<Clock>::getMicros()
	{
		auto elapsed = Clock::now() - mStart;
		return std::chrono::duration_cast<MicroSeconds>(elapsed);
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