#include "audioappeventhandler.h"
// Std includes
#include <thread>

namespace nap
{
	void AudioAppEventHandler::process()
	{
		{
			int ns = 0.5 * 1000000;
			std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
		}
	}

}