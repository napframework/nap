#pragma once

// External Includes
#include <chrono>

// Local Includes
#include "configure.h"

namespace nap
{
	/**
	 * Holds track of time based on a start point
	 * This timer is not threaded and doesn't work with callbacks
	 * it simply returns a time based on a start time
	 */
	class SimpleTimer
	{
	public:

		using ChronoClock = std::chrono::high_resolution_clock;
		using Milliseconds = std::chrono::milliseconds;
		using Seconds = std::chrono::seconds;
		using TimePoint = std::chrono::time_point<ChronoClock>;

		// Construction / Destruction
		SimpleTimer() = default;
		virtual ~SimpleTimer() = default;

		/**
		 * Start the timer
		 */
		void start();

		/**
		 * Stop the timer, resets state
		 */
		void stop();

		/**
		 * @return amount of processed ticks
		 */
		uint32 getTicks();

		/**
		 * @return the elapsed time in seconds as double
		 */
		double getElapsedTime();

	private:

		// Members
		TimePoint  mStart;	//< Start Time
	};
}
