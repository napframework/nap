/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/modulemanager.h>
#include <nap/android/androidextension.h>
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
	bool ModuleManager::loadModules(const std::vector<std::string>& moduleNames, utility::ErrorState& error)
	{
		// Track which modules remain to be loaded
		std::vector<std::string> modulesToLoad;
		if (!fetchProjectModuleDependencies(moduleNames, modulesToLoad, error))
			return false;
		
		for (const auto& moduleName : modulesToLoad)
		{
			rtti::TypeInfo service = rtti::TypeInfo::empty();
			const AndroidExtension& android_ext = mCore.getExtension<AndroidExtension>();
			std::string module_path = android_ext.getNativeLibDir() + "/lib" + moduleName + ".so";
			
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
			if (descriptor->mAPIVersion != nap::moduleAPIVersion)
			{
				Logger::warn("Module %s was built against a different version of NAP (found %d, expected %d); skipping.", module_path.c_str(), descriptor->mAPIVersion, nap::moduleAPIVersion);
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
		// We don't use module search directories on Android
		// TODO ANDROID Restructure API to not have this pointless stub?
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


	bool ModuleManager::fetchImmediateModuleDependencies(const std::vector<std::string>& searchModules, 
		                                                 std::vector<std::string>& previouslyFoundModules, 
		                                                 std::vector<std::string>& dependencies, 
		                                                 utility::ErrorState& errorState)
	{
		// List of remaining modules to locate dependencies for
		std::vector<std::string> remainingModulesToFind = searchModules;
		
		if (!errorState.check(mCore.hasExtension<AndroidExtension>(), "Core not setup with Android extension!"))
			return false;

		// Get interface
		const AndroidExtension& android_ext = mCore.getExtension<AndroidExtension>();

		// Iterate the directories in our search path
		for (const auto& moduleName : searchModules) 
		{
			// Get the path for the module.json
			std::string jsonFile = "nap_modules/" + moduleName + "/module.json";

			// Open the asset using Android's AssetManager
			// TODO ANDROID Cleanup, harden and code re-use
	        AAsset* asset = AAssetManager_open(android_ext.getAssetManager(), jsonFile.c_str(), AASSET_MODE_BUFFER);
	        if (asset == NULL) 
	        {
    			errorState.fail("AssetManager couldn't load asset %s", jsonFile.c_str());
	            return false;
	        }

	        // Read the asset
	        long size = AAsset_getLength(asset);
	        char* buffer = (char*) malloc (sizeof(char)*size);
	        AAsset_read(asset, buffer, size);
			std::string outBuffer(buffer, size);
	        AAsset_close(asset);

			// Load the dependencies from the JSON file
			std::vector<std::string> dependenciesFromJson;
			if (!deserializeModuleDependenciesFromJson(outBuffer, dependenciesFromJson, errorState))
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
