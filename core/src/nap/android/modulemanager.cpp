// Local Includes
#include <nap/modulemanager.h>
#include <nap/logger.h>
#include <nap/service.h>
#include <nap/module.h>
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	bool ModuleManager::loadModules(const std::vector<std::string>& moduleNames, utility::ErrorState& error, std::string forcedModuleDirectory)
	{
		// TODO ANDROID At the moment we're only supporting a top level specific list of modules to load;
		// we don't supporting module child dependencies pulled from module.json files and we don't
		// support the native behaviour which loads all available modules when none are specified

		// Track which modules remain to be loaded
		std::vector<std::string> modulesToLoad;
		if (!fetchProjectModuleDependencies(moduleNames, modulesToLoad, error))
			return false;
		
		for (const auto& moduleName : modulesToLoad)
		{
			rtti::TypeInfo service = rtti::TypeInfo::empty();

			std::string module_path = forcedModuleDirectory + "/lib" + moduleName + ".so";
			
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
			
			mModules.push_back(module);
		}

		// Fail if we haven't managed to load some of our requested modules
		if (mModules.size() != modulesToLoad.size())
		{
			std::string missingModulesToLog;
			for (const auto &missingModule : moduleNames)
				missingModulesToLog += missingModule + " ";
			error.fail("Failed to load requested modules: " + missingModulesToLog);
			return false;
		}

		return true;
	}


	bool ModuleManager::buildModuleSearchDirectories(const std::vector<std::string>& moduleNames, 
		                                             std::vector<std::string>& searchDirectories, 
		                                             utility::ErrorState& errorState)
	{
		// TODO ANDROID Not using module search directories for Android, permanent?
		return true;
	}


	bool ModuleManager::fetchProjectModuleDependencies(const std::vector<std::string>& topLevelProjectModules, 
		                                               std::vector<std::string>& dependencies, 
		                                               utility::ErrorState& errorState)
	{
		// TODO ANDROID temporary fetchProjectModuleDependencies? For now just use an explicit module list in project.json
		dependencies.insert(dependencies.end(), topLevelProjectModules.begin(), topLevelProjectModules.end());
		return true;
	}	
}
