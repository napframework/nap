// Local Includes
#include <nap/modulemanager.h>
#include <nap/logger.h>
#include <nap/service.h>
#include <nap/module.h>
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>
#include <dlfcn.h>
#include <nap/simpleserializer.h>

namespace nap
{
	bool ModuleManager::loadModules(const std::vector<std::string>& moduleNames, utility::ErrorState& error)
	{
		// Whether we're loading a specific set of requested modules
		const bool loadSpecificModules = moduleNames.size() != 0;

		// Track which modules remain to be loaded
		std::vector<std::string> remainingModulesToLoad;
		if (loadSpecificModules)
		{
			if (!fetchProjectModuleDependencies(moduleNames, remainingModulesToLoad, error))
				return false;
		}
		
		// Build a list of directories to search for modules
		std::vector<std::string> directories;
		if (!buildModuleSearchDirectories(remainingModulesToLoad, directories, error))
			return false;
		

		// Iterate each directory
		for (const auto& directory : directories) 
		{
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

	bool ModuleManager::loadModules(const ProjectInfo& projectInfo, utility::ErrorState& err)
	{
		for (const std::string& moduleName : projectInfo.mModuleNames)
		{
			if (!loadModule_(projectInfo, moduleName, err))
			{
				err.fail("Failed to load module '%s'", moduleName.c_str());
				return false;
			}
		}
		return true;
	}

	bool ModuleManager::buildModuleSearchDirectories(const std::vector<std::string>& moduleNames,
		                                             std::vector<std::string>& searchDirectories,
		                                             utility::ErrorState& errorState)
	{
#ifdef _WIN32
		// Windows
		searchDirectories.push_back(utility::getExecutableDir());
#elif defined(NAP_PACKAGED_BUILD)
		// Running against released NAP on macOS & Linux
		const std::string exeDir = utility::getExecutableDir();

		// Check if we're running a packaged project (on macOS or Linux)
		if (utility::fileExists(exeDir + "/lib/libnapcore." + getModuleExtension()))
		{
			searchDirectories.push_back(exeDir + "/lib");
		}
		// If we have no modules requested we're running in a non-project context, eg. napkin
		else if (moduleNames.size() == 0)
		{
			buildPackagedNapNonProjectModulePaths(searchDirectories);
		}
		else {
			std::vector<std::string> dirParts;
			utility::splitString(exeDir, '/', dirParts);
			// Check if we can see our build output folder, otherwise we're in an unexpected configuration
			if (folderNameContainsBuildConfiguration(dirParts.end()[-1]))
			{
				buildPackagedNapProjectModulePaths(moduleNames, searchDirectories);
			} else
			{
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
			searchDirectories.push_back(napRoot + "lib/" + full_configuration_name);

		} else
		{
			errorState.fail("Unexpected path configuration found, can't locate modules");
			return false;
		}
#endif // _WIN32
		return true;
	}

	bool ModuleManager::fetchProjectModuleDependencies(const std::vector<std::string>& topLevelProjectModules,
		                                               std::vector<std::string>& dependencies,
		                                               utility::ErrorState& errorState)
	{
		// Take the top level modules from project.json as the first modules to work on
		std::vector<std::string> newModules = topLevelProjectModules;

		// Work until we find loop and come across no new dependencies
		std::vector<std::string> searchModules;
		while (!newModules.empty())
		{
			// Add our new dependencies to our master list
			dependencies.insert(dependencies.end(), newModules.begin(), newModules.end());

			// Fetch any new dependencies for the dependencies found in the last loop
			searchModules = newModules;
			newModules.clear();
			if (!fetchImmediateModuleDependencies(searchModules, dependencies, newModules,
				                                  errorState))
				return false;
		}

		return true;
	}

	bool ModuleManager::loadModule_(const ProjectInfo& project, const std::string& moduleName, utility::ErrorState& err)
	{
		// Fail if we're trying to load the same module twice
		if (getLoadedModule(moduleName))
		{
			err.fail(utility::stringFormat("Module already loaded: %s", moduleName.c_str()));
			return false;
		}

		// Attempt to find the module files first
		std::string moduleFile;
		std::string moduleJson;
		if (!findModuleFiles(project, moduleName, moduleFile, moduleJson))
		{
			nap::Logger::error("Cannot find module %s", moduleName.c_str());
			return false;
		}

		// Load module json
		ModuleInfo modinfo;
		if (!modinfo.load(moduleJson, err))
			return false;

		// Load module dependencies first
		for (const auto& modName : modinfo.mDependencies)
		{
			if (getLoadedModule(modName))
				continue;

			if (!loadModule_(project, modName, err))
				return false;
		}

		// Load module binary
		void* module_handle = dlopen(moduleFile.c_str(), RTLD_LAZY);
		if (!module_handle)
		{
			err.fail(utility::stringFormat("Failed to load module '%s': %s", moduleFile.c_str(), dlerror()));
			return false;
		}

		// Find descriptor. If the descriptor wasn't found in the dll,
		// assume it's not actually a nap module and unload it again.
		auto descriptor = (ModuleDescriptor*)findSymbolInModule(module_handle, "descriptor");
		if (!descriptor)
		{
			unloadModule(module_handle);
			return false;
		}

		// Check that the module version matches, skip otherwise.
		if (descriptor->mAPIVersion != ModuleDescriptor::ModuleAPIVersion)
		{
			err.fail(utility::stringFormat("Module %s version mismatch (found %d, expected %d)",
										   moduleFile.c_str(), descriptor->mAPIVersion,
										   ModuleDescriptor::ModuleAPIVersion));
			unloadModule(module_handle);
			return false;
		}

		// Try to load service if one is defined
		rtti::TypeInfo service = rtti::TypeInfo::empty();
		if (descriptor->mService)
		{
			rtti::TypeInfo stype = rtti::TypeInfo::get_by_name(rttr::string_view(descriptor->mService));
			if (!stype.is_derived_from(RTTI_OF(Service)))
			{
				err.fail(utility::stringFormat("Module %s service descriptor %s is not a service",
											   moduleFile.c_str(), descriptor->mService));
				unloadModule(module_handle);
				return false;
			}
			service = stype;
		}

		// Load successful, store loaded module
		Module module;
		module.mName = moduleName;
		module.mDescriptor = descriptor;
		module.mInfo = modinfo;
		module.mHandle = module_handle;
		module.mService = service;
		mModules.emplace_back(module);
		nap::Logger::debug("Module loaded: %s", module.mDescriptor->mID);

		return true;
	}

	const ModuleManager::Module* ModuleManager::getLoadedModule(const std::string& moduleName)
	{
		for (const auto& mod : mModules)
			if (mod.mName == moduleName)
				return &mod;
		return nullptr;
	}

	bool ModuleManager::findModuleFiles(const ProjectInfo& projectInfo, const std::string& moduleName,
										std::string& moduleFile, std::string& moduleJson)
	{
		auto expectedFilename = utility::stringFormat("lib%s.so", moduleName.c_str());
		auto expectedJsonFile = utility::stringFormat("lib%s.json", moduleName.c_str());

		for (const auto& dir : projectInfo.getModuleDirectories())
		{
			moduleFile = utility::stringFormat("%s/%s", dir.c_str(), expectedFilename.c_str());
			moduleJson = utility::stringFormat("%s/%s", dir.c_str(), expectedJsonFile.c_str());
			if (utility::fileExists(moduleFile) and utility::fileExists(moduleJson))
				return true;
		}
		return false;
	}

	bool ModuleManager::fetchImmediateModuleDependencies(const std::vector<std::string>& searchModules, 
		                                                 std::vector<std::string>& previouslyFoundModules, 
		                                                 std::vector<std::string>& dependencies, 
		                                                 utility::ErrorState& errorState)
	{
		// Based on the modules we're searching on build a set of directories to locate them in
		std::vector<std::string> directories;
		if (!buildModuleSearchDirectories(searchModules, directories, errorState))
			return false;
		
		// List of remaining modules to locate dependencies for
		std::vector<std::string> remainingModulesToFind = searchModules;
		
		// Iterate the directories in our search path
		for (const auto& directory : directories) 
		{
			// Skip directory if it doesn't exist
			if (!utility::dirExists(directory))
				continue;
			
			// Find all files in the specified directory
			std::vector<std::string> files_in_directory;
			utility::listDir(directory.c_str(), files_in_directory);
		
			// Iterate the files in our search path
			for (const auto& filename : files_in_directory)
			{
				// Ignore directories
				if (utility::dirExists(filename))
					continue;
				
				// Ignore non-shared libraries
				if (!isModule(filename))
					continue;
				
				// Skip any modules we come across that aren't the ones we're after
				std::string moduleName = getModuleNameFromPath(filename);
				if (std::find(remainingModulesToFind.begin(), remainingModulesToFind.end(), 
					          moduleName) == remainingModulesToFind.end())
					continue;

				// Get the JSON filename for the module
				std::string modulePath = utility::getAbsolutePath(filename);
#if defined(_WIN32)
				std::replace(modulePath.begin(), modulePath.end(), '\\', '/');
#endif
				std::string jsonFile = utility::getFileDir(modulePath) + "/" + 
				                       utility::getFileNameWithoutExtension(modulePath);
				jsonFile = jsonFile + ".json";

				// Load the dependencies from the JSON file
				std::vector<std::string> dependenciesFromJson;
				if (!loadModuleDependenciesFromJsonFile(jsonFile, dependenciesFromJson, errorState))
					return false;

				// Iterate each dependency for this module
				for (const auto& dependencyModule : dependenciesFromJson)
				{
					// Check if we already had this dependency before listing dependencies for this module
					bool hadModulePreviously = std::find(previouslyFoundModules.begin(), previouslyFoundModules.end(), dependencyModule) != previouslyFoundModules.end();

					// Check if we already had added this dependency as a new dependency
					bool foundModuleRecently = std::find(dependencies.begin(), dependencies.end(), dependencyModule) != dependencies.end();
					
					// If we've found a new dependency add it to the list
					if (!hadModulePreviously && !foundModuleRecently)
						dependencies.push_back(dependencyModule);
				}
				
				// Remove our handled module from the remaining modules to load dependencies for
				std::vector<std::string>::iterator position = std::find(remainingModulesToFind.begin(), remainingModulesToFind.end(), moduleName);
				if (position != remainingModulesToFind.end())
					remainingModulesToFind.erase(position);
			}
			
			// At least break our outer loop if we've already loaded dependencies for the requested modules
			if (remainingModulesToFind.empty())
				break;
		}
		
		// If we couldn't load dependencies for all modules error and fail
		if (!remainingModulesToFind.empty())
		{
			std::string errorModulesToLog;
			for (const auto &errorModule : remainingModulesToFind)
				errorModulesToLog += errorModule + " ";
			errorState.fail("Couldn't load modules dependencies for: " + errorModulesToLog);
			return false;
		}
	
		return true;
	}		
}
