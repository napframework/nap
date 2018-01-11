// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "vinylapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <apprunner.h>

/**
 * Create and start the vinyl application. This app uses GUI components,
 * For that reason we use the GUIAppEventHandler instead of the default event handler
 * The GUI event handler blocks input targeted at the gui instead of the scene
 * This ensures the scene / viewport does not receive input events that are used to control gui components
 * For more information about the app refer to vinylapp.h
 */
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner
	nap::AppRunner<nap::VinylApp, nap::GUIAppEventHandler> app_runner(core);

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
