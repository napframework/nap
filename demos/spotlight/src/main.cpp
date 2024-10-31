// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "spotlightapp.h"

// Nap includes
#include <apprunner.h>
#include <nap/logger.h>
#include <guiappeventhandler.h>

// Main loop
int main(int argc, char *argv[])
{
    // Create core
    nap::Core core;

    // Create the application runner, based on the app to run
	// and event handler that is used to forward information into the app.
    nap::AppRunner<nap::SpotlightApp, nap::GUIAppEventHandler> app_runner(core);

    // Start running
    nap::utility::ErrorState error;
    if (!app_runner.start(error))
    {
        nap::Logger::fatal("error: %s", error.toString().c_str());
        return -1;
    }

    // Return if the app ran successfully
    return app_runner.exitCode();
}

