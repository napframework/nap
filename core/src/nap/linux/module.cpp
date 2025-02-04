/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/module.h>
#include <nap/logger.h>

// External Includes
#include <dlfcn.h> // Posix shared object loading
#include <linux/limits.h>

namespace nap
{
	void initModules()
	{ }


 	void* loadModule(const nap::ModuleInfo& modInfo, const std::string& library, std::string& outLocation, std::string& errorString)
	{
		// Attempt to load the library using default OS system mapping
		// If that fails attempt to locate it using library search paths
		void* handle = dlopen(library.c_str(), RTLD_LAZY);
		if (handle == nullptr)
		{
			for (const auto& path : modInfo.mLibSearchPaths)
			{
				// Find it
				std::string full_path = utility::forceSeparator(utility::stringFormat("%s/%s",
					path.c_str(), library.c_str()));
				if (!utility::fileExists(full_path))
					continue;

				// Load it
				Logger::debug("Explicitly loading library: %s", full_path.c_str());
				handle = dlopen(full_path.c_str(), RTLD_LAZY);
				if (handle != nullptr)
					break;
			}
		}

		// Ensure we have a handle
		if (handle == nullptr)
		{
			errorString = utility::stringFormat("dlopen for library '%s' failed:\n%s", library.c_str(), dlerror());
			return nullptr;
		}

		// Get library load origin
		char path[PATH_MAX];
		if (dlinfo(handle, RTLD_DI_ORIGIN, &path) == -1)
		{
			errorString = utility::stringFormat("origin resolve for '%s' failed:\n%s", library.c_str(), dlerror());
			return nullptr;
		}

		// Set out location
		outLocation = path;
		return handle;
	}


	void unloadModule(void* module)
	{
		dlclose(module);
	}


	void* findSymbolInModule(void* module, const char* symbolName)
	{
		return dlsym(module, symbolName);
	}


	std::string getModuleExtension()
	{
		return "so";
	}
}
