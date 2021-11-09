/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "computeflockingapp.h"

// External includes
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
	nap::AppRunner<nap::ComputeFlockingApp, nap::GUIAppEventHandler> app_runner(core);

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
