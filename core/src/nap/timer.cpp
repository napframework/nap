// Local Includes
#include "timer.h"

namespace nap
{
	// Start the timer
	void SimpleTimer::start()
	{
		mStart = ChronoClock::now();
	}


	// return start time
	TimePoint SimpleTimer::getStartTime() const
	{
		return mStart;
	}


	// Stop timer
	void SimpleTimer::stop()
	{
		mStart = TimePoint(Milliseconds(0));
	}


	// Reset the timer
	void SimpleTimer::reset()
	{
		start();
	}

	// Return number of ticks in milliseconds
	uint32 SimpleTimer::getTicks() const
	{
		std::chrono::duration<double> elapsed = ChronoClock::now() - mStart;
		return std::chrono::duration_cast<Milliseconds>(elapsed).count();
	}


	// Return elapsed time in seconds
	double SimpleTimer::getElapsedTime() const
	{
		return std::chrono::duration<double>(ChronoClock::now() - mStart).count();
	}


	// Elapsed time in seconds
	float SimpleTimer::getElapsedTimeFloat() const
	{
		return std::chrono::duration<float>(ChronoClock::now() - mStart).count();
	}


	// Return current time
	extern TimePoint getCurrentTime()
	{
		return ChronoClock::now();
	}

}