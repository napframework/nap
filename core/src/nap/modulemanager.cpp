// Local Includes
#include "modulemanager.h"
#include "logger.h"
#include "service.h"
#include "module.h"

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>

namespace nap
{
	ModuleManager::ModuleManager()
	{
		initModules();
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
		// Build a list of directories to search for modules
		std::vector<std::string> directories;
		if (!buildModuleSearchDirectories(moduleNames, directories, error))
			return false;

		// Whether we're loading a specific set of requested modules
		const bool loadSpecificModules = moduleNames.size() != 0;
		
		// Track which modules remain to be loaded
		std::vector<std::string> remainingModulesToLoad = moduleNames;
		
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
				if (!isModule(filename))
					continue;
				
				// Skip unrequested modules if we've been specified a list to load
				// First get our module name from the filename, removing 'lib' prefix on *nix
				std::string moduleName = getModuleNameFromPath(filename);
				if (loadSpecificModules && std::find(remainingModulesToLoad.begin(), remainingModulesToLoad.end(), moduleName) == remainingModulesToLoad.end())
					continue;

				std::string module_path = utility::getAbsolutePath(filename);

				// Try to load the module
				std::string error_string;
				void* module_handle = loadModule(module_path, error_string);
				if (!module_handle)
				{
					Logger::warn("Failed to load module %s: %s", module_path.c_str(), error_string.c_str());
					continue;
				}

				// Find descriptor. If the descriptor wasn't found in the dll, assume it's not actually a nap module and unload it again.
				ModuleDescriptor* descriptor = (ModuleDescriptor*)findSymbolInModule(module_handle, "descriptor");
				if (descriptor == nullptr)
				{
					unloadModule(module_handle);
					continue;
				}

				// Check that the module version matches, skip otherwise.
				if (descriptor->mAPIVersion != ModuleDescriptor::ModuleAPIVersion)
				{
					Logger::warn("Module %s was built against a different version of NAP (found %d, expected %d); skipping.", module_path.c_str(), descriptor->mAPIVersion, ModuleDescriptor::ModuleAPIVersion);
					unloadModule(module_handle);
					continue;
				}

				// Try to load service if one is defined
				if (descriptor->mService != nullptr)
				{
					rtti::TypeInfo stype = rtti::TypeInfo::get_by_name(rttr::string_view(descriptor->mService));
					if (!stype.is_derived_from(RTTI_OF(Service)))
					{
						Logger::warn("Module %s service descriptor %s is not a service; skipping", module_path.c_str(), descriptor->mService);
						unloadModule(module_handle);
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
				
				// If we're tracking a specific set of modules to load, remove the loaded module from our list.  Used
				// to report on any missing modules.
				if (loadSpecificModules)
					remainingModulesToLoad.erase(std::remove(remainingModulesToLoad.begin(), remainingModulesToLoad.end(), moduleName), remainingModulesToLoad.end());

				mModules.push_back(module);
			}
		}
		
		// Fail if we haven't managed to load some of our requested modules
		if (loadSpecificModules && remainingModulesToLoad.size() > 0)
		{
			std::string missingModulesToLog;
			for (const auto &missingModule : moduleNames)
				missingModulesToLog += missingModule + " ";
			error.fail("Failed to load requested modules: " + missingModulesToLog);
			return false;
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
		searchDir = napRoot + "user_modules";
		filesInDirectory.clear();
		utility::listDir(searchDir.c_str(), filesInDirectory, false);
		for (const std::string& module: filesInDirectory)
		{
			std::string dirName = napRoot + "user_modules/" + module + "/lib/" + buildType;
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
			outSearchDirectories.push_back(napRoot + "user_modules/" + module + "/lib/" + buildType);
		}
		
		// Project module
		outSearchDirectories.push_back(exeDir + "/../../module/lib/" + buildType);
	}
}
