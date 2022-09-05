/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "pipinsapp.h"

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
    nap::AppRunner<nap::CoreApp, nap::GUIAppEventHandler> app_runner(core);

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

