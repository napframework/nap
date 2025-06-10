/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "projectinfo.h"
#include <string>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Performs any platform specific module initialization.
	 */
	NAPAPI void initModules();

	/**
	 * Platform-wrapper function to load a shared library
	 * @param modInfo The moduleInfo, used to add additional dll search paths
	 * @param library The name of the library to load, including extension
	 * @param outLocation the location the loaded module, if loading succeeds
	 * @param error Holds the error if loading fails
	 * @return Handle to the loaded module
	 */
	NAPAPI void* loadModule(const nap::ModuleInfo& modInfo, const std::string& library, std::string& outLocation, std::string& error);

	/**
	 * Platform-wrapper function to unload a shared library
	 * @param module The module to load
	 */
	NAPAPI void unloadModule(void* module);

	/**
	 * Platform-wrapper function to find the address of a symbol in the specified shared library
	 * @param module Handle to the module to find the symbol in
	 * @param symbolName Name of the symbol to find
	 * @return The pointer to the symbol if found, nullptr if not
	 */
	NAPAPI void* findSymbolInModule(void* module, const char* symbolName);

	/**
	 * @return Module filename extension for platform
	 */
	NAPAPI std::string getModuleExtension();
}
