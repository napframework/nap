#include <iostream>

// Std includes
#include <thread>
#include <atomic>

// Local includes
#include "audiotestapp.h"
#include "audioappeventhandler.h"

// Nap includes
#include <utility/dllexport.h>
#include <nap/core.h>
#include <nap/logger.h>

#include <pybind11/pybind11.h>

#include <apprunner.h>

// Main loop
int main(int argc, char *argv[])
{
    // Create core
    nap::Core core;

    // Create app runner
    nap::AppRunner<nap::AudioTestApp, nap::AudioAppEventHandler> app_runner(core);

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

     
