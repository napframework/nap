#include "../module.h"
#include <utility/fileutils.h>

#include <dlfcn.h> // Posix shared object loading

namespace nap
{
	void initModules()
	{
		// On windows, disable DLL load failure dialog boxes (we handle the errors ourselves))
		SetErrorMode(SEM_FAILCRITICALERRORS);
	}


	void* loadModule(const std::string& modulePath, std::string& errorString)
	{
		void* result;
		result = dlopen(modulePath.c_str(), RTLD_LAZY);

		// If we failed to load the module, get the error string
		if (!result)
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


	std::string getModuleNameFromPath(const std::string& path)
	{
		assert(isModule(path));
		std::string moduleName = utility::getFileNameWithoutExtension(path);
		assert(moduleName.substr(0, 3) == "lib");
		moduleName = moduleName.substr(3, std::string::npos);
		return moduleName;
	}


	NAPAPI bool isModule(const std::string& path)
	{
		return utility::getFileExtension(path) == "dylib";
	}
}