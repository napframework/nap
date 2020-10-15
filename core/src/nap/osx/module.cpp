/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/module.h>

// External Includes
#include <utility/fileutils.h>
#include <assert.h>

#include <dlfcn.h> // Posix shared object loading

namespace nap
{
	void initModules()
	{ }


 	void* loadModule(const nap::ModuleInfo& modInfo, const std::string& modulePath, std::string& errorString)
	{
		// If we failed to load the module, get the error string
		void* result = dlopen(modulePath.c_str(), RTLD_LAZY);
		if (result == nullptr)
			errorString = dlerror();
		
		return result;
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
		return "dylib";
	}
}
