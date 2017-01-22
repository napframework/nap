// Local Includes
#include "timer.h"

namespace nap
{
	// Start the timer
	void SimpleTimer::start()
	{
		mStart = ChronoClock::now();
	}


	// Stop timer
	void SimpleTimer::stop()
	{
		mStart = TimePoint(Milliseconds(0));
	}


	// Return number of ticks in milliseconds
	uint32 SimpleTimer::getTicks()
	{
		std::chrono::duration<double> elapsed = ChronoClock::now() - mStart;
		return std::chrono::duration_cast<Milliseconds>(elapsed).count();
	}


	// Return elapsed time in seconds
	double SimpleTimer::getElapsedTime()
	{
		return std::chrono::duration<double>(ChronoClock::now() - mStart).count();
	}
}