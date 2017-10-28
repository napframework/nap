// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "etherdreamapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <apprunner.h>

// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner
	nap::AppRunner<nap::EtherdreamApp, nap::AppEventHandler> app_runner(core);

	// Start
	nap::utility::ErrorState error;
	if (!app_runner.start(error))
	{
		nap::Logger::fatal("error: %s", error.toString().c_str());
		return -1;
	}

	// Return if the app ran successfully
	return app_runner.exitCode();
}
     
