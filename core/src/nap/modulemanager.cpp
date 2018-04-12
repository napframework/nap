// Local Includes
#include "modulemanager.h"
#include "logger.h"
#include "service.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h> // Windows dll loading
#else
	#include <dlfcn.h> // Posix shared object loading
#endif

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>

namespace
{
	// Extension for shared libraries (dependent on platform)
#ifdef _WIN32
	static std::string sharedLibExtension = "dll";
#elif __APPLE__
	static std::string sharedLibExtension = "dylib";
#else
	static std::string sharedLibExtension = "so";
#endif


	/**
	 * Platform-wrapper function to get the error string for loading a module (if an error was thrown)
	 * @return The error string
	 */
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


	/**
	 * Platform-wrapper function to load a shared library
	 * @param modulePath The path to the shared library to load
	 * @param errorString If the module failed to load, the error string
	 * @return Handle to the loaded module
	 */
	void* LoadModule(const std::string& modulePath, std::string& errorString)
	{
		void* result;
#ifdef _WIN32
		result = LoadLibraryA(modulePath.c_str());
#else
		result = dlopen(modulePath.c_str(), RTLD_LAZY);
#endif

		// If we failed to load the module, get the error string
		if (!result)
			errorString = GetModuleLoadErrrorString();

		return result;
	}


	/**
	 * Platform-wrapper function to unload a shared library
	 * @param module The module to load
	 */
	void UnloadModule(void* module)
	{
#ifdef _WIN32
		FreeLibrary((HMODULE)module);
#else
		dlclose(module);
#endif
	}


	/**
	 * Platform-wrapper function to find the address of a symbol in the specified shared library
	 * @param module Handle to the module to find the symbol in
	 * @param symbolName Name of the symbol to find
	 * @return The pointer to the symbol if found, nullptr if not
	 */
	void* FindSymbol(void* module, const char* symbolName)
	{
#ifdef _WIN32
		return GetProcAddress((HMODULE)module, symbolName);
#else
		return dlsym(module, symbolName);
#endif
	}
}

namespace nap
{
	ModuleManager::ModuleManager()
	{
#ifdef _WIN32
		// On windows, disable DLL load failure dialog boxes (we handle the errors ourselves))
		SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
	}

	ModuleManager::~ModuleManager()
	{
		/*  Commented out for now because unloading modules can cause crashes in RTTR; during shutdown it will try
			to access RTTI types registered by potentially unloaded modules and crash because the pointers are no longer valid.
			
			This should probably be fixed in RTTR itself, but I'm not sure how yet. For now I've disabled the unloading of modules, 
			since this only happens during shutdown and modules will be unloaded then anyway.
		
 		for (Module& module : mModules)
 			UnloadModule(module.mHandle);
		*/
	}

	bool ModuleManager::loadModules(std::vector<std::string>& moduleNames, utility::ErrorState& error)
	{
		/** 
		 * TODO Discuss changing loadModules to provide these two modes of operation:
		 *   - A standard/project mode where it only loads the modules specified in the provided list and fails if
		 *     any of those modules couldn't be loaded
		 *   - A 'load everything' mode for Napkin or sandbox NAP use where no modules are specified and we load 
		 *     everything we encounter
		 */
		
		// Build a list of directories to search for modules
		std::vector<std::string> directories;
		if (!buildModuleSearchDirectories(moduleNames, directories, error))
			return false;

		// Iterate each directory
		for (const auto& directory : directories) {
			// Skip directory if it doesn't exist
			if (!utility::dirExists(directory))
				continue;

			// Find all files in the specified directory
			std::vector<std::string> files_in_directory;
			utility::listDir(directory.c_str(), files_in_directory);

			for (const auto& filename : files_in_directory)
			{
				rtti::TypeInfo service = rtti::TypeInfo::empty();

				// Ignore directories
				if (utility::dirExists(filename))
					continue;

				// Ignore non-shared libraries
				if (utility::getFileExtension(filename) != sharedLibExtension)
					continue;

				std::string module_path = utility::getAbsolutePath(filename);

				// Try to load the module
				std::string error_string;
				void* module_handle = LoadModule(module_path, error_string);
				if (!module_handle)
				{
					Logger::warn("Failed to load module %s: %s", module_path.c_str(), error_string.c_str());
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
					Logger::warn("Module %s was built against a different version of NAP (found %d, expected %d); skipping.", module_path.c_str(), descriptor->mAPIVersion, ModuleDescriptor::ModuleAPIVersion);
					UnloadModule(module_handle);
					continue;
				}

				// Try to load service if one is defined
				if (descriptor->mService != nullptr)
				{
					rtti::TypeInfo stype = rtti::TypeInfo::get_by_name(rttr::string_view(descriptor->mService));
					if (!stype.is_derived_from(RTTI_OF(Service)))
					{
						Logger::warn("Module %s service descriptor %s is not a service; skipping", module_path.c_str(), descriptor->mService);
						UnloadModule(module_handle);
						continue;
					}
					service = stype;
				}

				Logger::debug("Loaded module %s v%s", descriptor->mID, descriptor->mVersion);

				// Construct module based on found module information
				Module module;
				module.mDescriptor = descriptor;
				module.mHandle = module_handle;
				module.mService = service;

				mModules.push_back(module);
			}
		}
		return true;
	}
	

	bool ModuleManager::buildModuleSearchDirectories(std::vector<std::string>& moduleNames, std::vector<std::string>& outSearchDirectories, utility::ErrorState& errorState)
	{
#ifdef _WIN32
		// Windows
		outSearchDirectories.push_back(utility::getExecutableDir());
#elif defined(NAP_PACKAGED_BUILD)
		// Running against released NAP on macOS & Linux
		const std::string exeDir = utility::getExecutableDir();
		
		// Check if we're running a packaged project (on macOS or Linux)
		if (utility::fileExists(exeDir + "/lib/libnapcore." + sharedLibExtension))
		{
			outSearchDirectories.push_back(exeDir + "/lib");
		}
		// If we have no modules requested we're running in a non-project context, eg. napkin
		else if (moduleNames.size() == 0)
		{
			buildPackagedNapNonProjectModulePaths(outSearchDirectories);
		}
		else {
			std::vector<std::string> dirParts;
			utility::splitString(exeDir, '/', dirParts);
			// Check if we can see our build output folder, otherwise we're in an unexpected configuration
			if (folderNameContainsBuildConfiguration(dirParts.end()[-1]))
			{
				buildPackagedNapProjectModulePaths(moduleNames, outSearchDirectories);
			}
			else {
				errorState.fail("Unexpected path configuration found, can't locate modules");
				return false;
			}
		}
#else
		Logger::debug("Running from NAP source");
	
		const std::string exeDir = utility::getExecutableDir();

		std::vector<std::string> dirParts;
		utility::splitString(exeDir, '/', dirParts);
		// Check if we can see our build output folder, otherwise we're in an unexpected configuration
		if (dirParts.size() > 1 && folderNameContainsBuildConfiguration(dirParts.end()[-1]))
		{
			// MacOS & Linux apps in NAP internal source
			const std::string napRoot = exeDir + "/../../";
			const std::string full_configuration_name = dirParts.end()[-1];
			outSearchDirectories.push_back(napRoot + "lib/" + full_configuration_name);

		} else {
			errorState.fail("Unexpected path configuration found, can't locate modules");
			return false;
		}
#endif // _WIN32
		return true;
	}
	

	bool ModuleManager::folderNameContainsBuildConfiguration(std::string& folderName)
	{
		std::vector<std::string> configParts;
		utility::splitString(folderName, '-', configParts);
		// If we don't have enough parts we're not looking at a build folder
		if (configParts.size() != 3)
			return false;
		
#if defined(__APPLE__) || defined(_WIN32)
		const std::string folderBuildType = configParts[2];
#elif __unix__
		const std::string folderBuildType = configParts[1];
#endif

#ifdef NDEBUG
		const std::string runningBuildType = "Release";
#else
		const std::string runningBuildType = "Debug";
#endif
	
		if (runningBuildType != folderBuildType)
			return false;
		
		return true;
	}
	

	void ModuleManager::buildPackagedNapNonProjectModulePaths(std::vector<std::string>& outSearchDirectories)
	{
		// Cater for Napkin running against packaged NAP.  Module names won't be specified.  Let's load everything we can find.
		
#ifdef NDEBUG
		const std::string buildType = "Release";
#else
		const std::string buildType = "Debug";
#endif
		
		std::string exeDir = utility::getExecutableDir();
		std::string napRoot = exeDir + "/../../../../";
		
		// NAP modules
		std::string searchDir = napRoot + "modules";
		std::vector<std::string> filesInDirectory;
		utility::listDir(searchDir.c_str(), filesInDirectory, false);
		for (const std::string& module: filesInDirectory)
		{
			std::string dirName = napRoot + "modules/" + module + "/lib/" + buildType;
			if (!utility::dirExists(dirName))
				continue;
			outSearchDirectories.push_back(dirName);
			//Logger::info("Adding module search path %s for napkin", dirName.c_str());
		}
		
		// User modules
		searchDir = napRoot + "usermodules";
		filesInDirectory.clear();
		utility::listDir(searchDir.c_str(), filesInDirectory, false);
		for (const std::string& module: filesInDirectory)
		{
			std::string dirName = napRoot + "usermodules/" + module + "/lib/" + buildType;
			if (!utility::dirExists(dirName))
				continue;
			outSearchDirectories.push_back(dirName);
			//Logger::info("Adding user module search path %s for napkin", dirName.c_str());
		}

		// Project module
		outSearchDirectories.push_back(exeDir + "/../../module/lib/" + buildType);
	}

	
	void ModuleManager::buildPackagedNapProjectModulePaths(std::vector<std::string>& moduleNames, std::vector<std::string>& outSearchDirectories)
	{
#ifdef NDEBUG
		const std::string buildType = "Release";
#else
		const std::string buildType = "Debug";
#endif
		
		std::string exeDir = utility::getExecutableDir();
		std::string napRoot = exeDir + "/../../../../";
		// Normal project running against packaged NAP
		for (const std::string& module : moduleNames)
		{
			// NAP modules
			outSearchDirectories.push_back(napRoot + "modules/" + module + "/lib/" + buildType);
			// User modules
			outSearchDirectories.push_back(napRoot + "usermodules/" + module + "/lib/" + buildType);
		}
		
		// Project module
		outSearchDirectories.push_back(exeDir + "/../../module/lib/" + buildType);
	}
}
