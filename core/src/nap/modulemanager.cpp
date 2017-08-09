#include "modulemanager.h"
#include "fileutils.h"
#include "logger.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h> // Windows dll loading
#else
	#include <dlfcn.h> // Posix shared object loading
#endif

namespace
{
#ifdef _WIN32
	static std::string moduleExtension = "dll";
#elif __APPLE__
	static std::string moduleExtension = "dylib";
#else
	static std::string moduleExtension = "so";
#endif

	std::string GetModuleLoadErrrorString()
	{
#ifdef _WIN32
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
#else
		return dlerror();
#endif
	}

	void* LoadModule(const std::string& modulePath, std::string& errorString)
	{
		void* result;
#ifdef _WIN32
		result = LoadLibraryA(modulePath.c_str());
#else
		result = dlopen(modulePath.c_str(), RTLD_LAZY);
#endif

		if (!result)
			errorString = GetModuleLoadErrrorString();

		return result;
	}

	void UnloadModule(void* module)
	{
#ifdef _WIN32
		FreeLibrary((HMODULE)module);
#else
		dlclose(module, RTLD_LAZY);
#endif
	}

	void* FindSymbol(void* module, const char* inSymbolName)
	{
#ifdef _WIN32
		return GetProcAddress((HMODULE)module, inSymbolName);
#else
		return dlsym(module, inSymbolName);
#endif
	}
}

namespace nap
{
	ModuleManager::ModuleManager()
	{
#ifdef _WIN32
		SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
	}

	ModuleManager::~ModuleManager()
	{
		for (Module& module : mModules)
			UnloadModule(module.mHandle);
	}

	void ModuleManager::loadModules(const std::string& directory)
	{
		std::vector<std::string> files_in_directory;
		nap::listDir(directory.c_str(), files_in_directory);

		for (const auto& filename : files_in_directory)
		{
			// Ignore directories
			if (dirExists(filename))
				continue;

			// Ignore non-shared libraries
			if (getFileExtension(filename) != moduleExtension)
				continue;

			std::string module_path = getAbsolutePath(filename);
			
			std::string error_string;
			void* module_handle = LoadModule(module_path, error_string);
			if (!module_handle)
			{
				Logger::info("Failed to load module %s: %s", module_path.c_str(), error_string.c_str());
				continue;
			}

			// Find descriptor. If the descriptor wasn't found in the dll, assume it's not actually a nap module and unload it again.
			ModuleDescriptor* descriptor = (ModuleDescriptor*)FindSymbol(module_handle, "descriptor");
			if (descriptor == nullptr)
			{
				UnloadModule(module_handle);
				continue;
			}

			// Check that the module version matches, skip otherwise.
			if (descriptor->mAPIVersion != ModuleDescriptor::ModuleAPIVersion)
			{
				Logger::info("Module %s was built against a different version of nap (found %d, expected %d); skipping.", module_path.c_str(), descriptor->mAPIVersion, ModuleDescriptor::ModuleAPIVersion);
				UnloadModule(module_handle);				
				continue;
			}

			Logger::info("Loaded module %s v%s", descriptor->mID, descriptor->mVersion);

			Module module;
			module.mDescriptor = descriptor;
			module.mHandle = module_handle;

			mModules.push_back(module);
		}
	}
}