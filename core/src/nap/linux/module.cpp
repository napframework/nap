#include "../module.h"
#include <utility/fileutils.h>
#include <assert.h>

#include <dlfcn.h> // Posix shared object loading

namespace nap
{
	void initModules()
	{
	}

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
		return "so";
	}
}
