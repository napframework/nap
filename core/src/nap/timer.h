#pragma once

// External Includes
#include <chrono>

// Local Includes
#include "configure.h"
#include "utility/dllexport.h"

namespace nap
{
	// Typedefs
	using ChronoClock = std::chrono::high_resolution_clock;
	using Milliseconds = std::chrono::milliseconds;
	using NanoSeconds = std::chrono::nanoseconds;
	using Seconds = std::chrono::seconds;
	using TimePoint = std::chrono::time_point<ChronoClock>;

	// Global get current time function
	extern TimePoint getCurrentTime();

	/**
	 * Holds track of time based on a start point
	 * This timer is not threaded and doesn't work with callbacks
	 * it simply returns a time based on a start time
	 */
	class NAPAPI SimpleTimer
	{
	public:
		// Construction / Destruction
		SimpleTimer() = default;
		virtual ~SimpleTimer() = default;

		/**
		 * Start the timer
		 */
		void start();

		/**
		 * @return start time as point in time
		 */
		TimePoint getStartTime() const;

		/**
		 * Stop the timer, resets state
		 */
		void stop();

		/**
		 *	Resets the timer and starts it again
		 */
		void reset();

		/**
		 * @return amount of processed ticks
		 */
		uint32 getTicks() const;

		/**
		 * @return the elapsed time in seconds as double
		 */
		double getElapsedTime() const;

		/**
		 * @return the elapsed time in seconds as a float
		 */
		float getElapsedTimeFloat() const;

	private:

		// Members
		TimePoint  mStart;	//< Start Time
	};
}
