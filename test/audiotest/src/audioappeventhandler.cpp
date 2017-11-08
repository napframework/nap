#include "audioappeventhandler.h"
// Std includes
#include <thread>

namespace nap
{
	void AudioAppEventHandler::process()
	{
        // sleep for 1 ms
        int ns = 1000000;
        std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
	}

}
