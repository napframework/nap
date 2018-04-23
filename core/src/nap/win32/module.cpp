#include "../module.h"
#include <utility/fileutils.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h> // Windows dll loading

namespace nap
{
	void initModules()
	{
		// On windows, disable DLL load failure dialog boxes (we handle the errors ourselves))
		SetErrorMode(SEM_FAILCRITICALERRORS);
	}


	std::string getModuleLoadErrorString()
	{
		// Get the error code
		DWORD error_code = ::GetLastError();
		if (error_code == 0)
			return std::string();

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);
		LocalFree(messageBuffer);

		return message;
	}


	void* loadModule(const std::string& modulePath, std::string& errorString)
	{
		void* result;
		result = LoadLibraryA(modulePath.c_str());

		// If we failed to load the module, get the error string
		if (!result)
			errorString = getModuleLoadErrorString();

		return result;
	}


	void unloadModule(void* module)
	{
		FreeLibrary((HMODULE)module);
	}


	void* findSymbolInModule(void* module, const char* symbolName)
	{
		return GetProcAddress((HMODULE)module, symbolName);
	}


	std::string getModuleNameFromPath(const std::string& path)
	{
		return utility::getFileNameWithoutExtension(path);
	}


	NAPAPI bool isModule(const std::string& path)
	{
		return utility::getFileExtension(path) == getModuleExtension();
	}
	
	
	std::string getModuleExtension()
	{
		return "dll";
	}
}