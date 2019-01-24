// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "emographyandroidapp.h"

// Nap includes
#include <nap/logger.h>
#include <servicerunner.h>
#include <appeventhandler.h>

#include <thread>

	int main(int argc, char *argv[])
	{
		// Create core
		nap::Core core;

		// Create app runner using default event handler
		nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler> service_runner(core);
		nap::EmographyAndroidApp& app = service_runner.getApp();

		nap::utility::ErrorState error;
		if (!service_runner.init(error))
		{
			nap::Logger::fatal("error: %s", error.toString().c_str());
			return -1;
		}

		while (true)
		{
			// Call our temporary methods
			service_runner.update();
			app.call("data");
			nap::Logger::info(app.pullLogAndFlush());
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

    return service_runner.shutdown();
}
