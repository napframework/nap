// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "videoapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <apprunner.h>

// TODO temp, for module paths
#include <utility/fileutils.h>

// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Create app runner
	nap::AppRunner<nap::VideoApp, nap::AppEventHandler> app_runner(core);
	
	
	// TODO this is very much work(arounds) in progress.. nothing to see here -------------------------------------------------------

	std::vector<std::string> moduleSearchDirectories;
	moduleSearchDirectories.push_back("."); // Packaged Win64 apps
	moduleSearchDirectories.push_back("lib"); // Packaged MacOS & Linux apps
	
	// MacOS & Linux apps in NAP internal source
	moduleSearchDirectories.push_back("../../lib/" + nap::utility::getFileName(nap::utility::getExecutableDir()));

	// TODO load from project JSON
	static std::vector<std::string> modules = {
		"mod_naprender",
		"mod_napmath",
		"mod_napinput",
		"mod_napsdlinput",
		"mod_napsdlwindow",
		"mod_napvideo",
		"mod_napapp",
		"mod_napscene"
	};

	// Building against NAP release
	std::string modulePathConfigSuffix;
#ifdef NDEBUG
	modulePathConfigSuffix = "Release";
#else
	modulePathConfigSuffix = "Debug";
#endif
	for (std::string& module : modules) {
		moduleSearchDirectories.push_back("../../../../modules/" + module + "/lib/" + modulePathConfigSuffix);
	}

	// ------------------------------------------------------------------------------------------------------------------------------
	
	// Start
	nap::utility::ErrorState error;
	if (!app_runner.start(moduleSearchDirectories, error))
	{
		nap::Logger::fatal("error: %s", error.toString().c_str());
		return -1;
	}

	// Return if the app ran successfully
	return app_runner.exitCode();
}



     
