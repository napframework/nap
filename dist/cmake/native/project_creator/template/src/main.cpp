// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "@PROJECT_NAME_LOWERCASE@app.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <apprunner.h>
#include <guiappeventhandler.h>

// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner
	nap::AppRunner<nap::@PROJECT_NAME_PASCALCASE@App, nap::GUIAppEventHandler> app_runner(core);

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
