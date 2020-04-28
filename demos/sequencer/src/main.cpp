// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "sequencerapp.h"

// Nap includes
#include <apprunner.h>
#include <nap/logger.h>
#include <guiappeventhandler.h>

// Main loop
int main(int argc, char *argv[])
{
    // Create core
    nap::Core core;

    // Create app runner
    nap::AppRunner<nap::SequencerApp, nap::GUIAppEventHandler> appRunner(core);

    // Decide which file to load
	appRunner.getApp().setFilename(argc >= 2 ? argv[1] : "default.json");

    // Start
    nap::utility::ErrorState error;
    if (!appRunner.start(error))
    {
        nap::Logger::fatal("error: %s", error.toString().c_str());
        return -1;
    }

    // Return if the app ran successfully
    return appRunner.exitCode();
}

