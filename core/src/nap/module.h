#pragma once

#include <utility/dllexport.h>
#include <string>

namespace nap
{
	/**
	 * Performs any platform specific module initialization.
	 */
	NAPAPI void initModules();

	/**
	 * Platform-wrapper function to load a shared library
	 * @param modulePath The path to the shared library to load
	 * @param errorString If the module failed to load, the error string
	 * @return Handle to the loaded module
	 */	NAPAPI void* loadModule(const std::string& modulePath, std::string& errorString);

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
	 * @param path Path to the module
	 * @return Name of the module
	 */
	NAPAPI std::string getModuleNameFromPath(const std::string& path);

	/**
	 * @param path Path to the module
	 * @return Whether the path points to a module for this platform.
	 */
	NAPAPI bool isModule(const std::string& path);

	/**
	 * @return Module filename extension for platform
	 */
	NAPAPI std::string getModuleExtension();
}
