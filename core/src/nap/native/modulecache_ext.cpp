/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/modulecache.h>
#include <nap/logger.h>
#include <nap/module.h>
#include <utility/module.h>
#include <nap/core.h>

// External Includes
#include <rtti/jsonreader.h>
#include <utility/fileutils.h>

namespace nap
{
	/**
	 * Given a ProjectInfo descriptor and a module name, retrieve the filename of the module and its json descriptor
	 * @param projectInfo The current project info descriptor we're using for this session
	 * @param moduleName The name of the module to find
	 * @param moduleFile The absolute path to resulting module
	 * @param moduleJson The absolute path to the resulting module json descriptor
	 * @return True if the operation was successful, false otherwise
	 */
	static std::string findModuleJSON(const ProjectInfo& projectInfo, const std::string& moduleName, utility::ErrorState& err)
	{
		// Require module directories to be provided
		auto module_dirs = projectInfo.getModuleDirectories();
		if (!err.check(!module_dirs.empty(), "No module dirs specified in %s", projectInfo.getFilename().c_str()))
			return std::string();

		// Substitute module name in given directories
		utility::namedFormat(module_dirs, { { "MODULE_NAME", moduleName } });

		// Find module json in given directories
		auto search_file = utility::stringFormat("%s.json", moduleName.c_str());
		auto module_json = utility::findFileInDirectories(search_file, module_dirs);
		if (!err.check(!module_json.empty(), "Module descriptor '%s' not found in any of these dirs:\n%s",
			search_file.c_str(), utility::joinString(module_dirs, "\n").c_str()))
			return std::string();

		return module_json;
	}


	static bool addDependency(std::vector<const nap::Module*>& dependencies, const nap::Module* candidate)
	{
		// Check if candidate isn't loaded already
		auto loaded = std::find_if(dependencies.begin(), dependencies.end(), [&candidate](const auto& it) {
				return it == candidate;
			});

		// Insert when not available
		if (loaded == dependencies.end()) 
		{
			dependencies.emplace_back(candidate);
			return true;
		}
		return false;
	}


	const nap::Module* ModuleCache::sourceModule(const ProjectInfo& project, const std::string& moduleName, utility::ErrorState& err)
	{
		// Return already cached module when already sourced
		const auto* cached_module = findModule(moduleName);
		if (cached_module != nullptr)
			return cached_module;

		// Attempt to find the module json descriptor first
		std::string module_json = findModuleJSON(project, moduleName, err);
		if(module_json.empty())
			return nullptr;

		// Load module json
		nap::rtti::Factory factory;
		auto module_info = rtti::getObjectFromJSONFile<nap::ModuleInfo>(
			module_json,
			rtti::EPropertyValidationMode::AllowMissingProperties,
			factory,
			err);

		if (!err.check(module_info != nullptr, "Failed to read %s from %s",
			RTTI_OF(nap::ModuleInfo).get_name().data(),  module_json.c_str()))
			return nullptr;

		// Store useful references so we can backtrack if necessary
		module_info->mProjectInfo = &project;

		// Add project directory default search path for modules, used by Windows packaged apps
		module_info->mLibSearchPaths.insert(module_info->mLibSearchPaths.begin(),
			{ utility::getFileDir(module_json), project.getProjectDir() });

		// Patch library search paths
		std::string module_dir = utility::getFileDir(module_json);
		project.patchPaths(module_info->mLibSearchPaths, {{"MODULE_DIR", module_dir}});

		// Patch library data paths
		project.patchPaths(module_info->mDataSearchPaths,
			{
				{"MODULE_NAME", moduleName},
				{"MODULE_DIR",  module_dir}
			});

		// Debug print data search paths
		for (const auto& data_path : module_info->mDataSearchPaths)
			nap::Logger::debug("Adding data search path: %s", data_path.c_str());

		// Recursively load module dependencies first
		std::vector<const nap::Module*> module_deps;
		for (const auto& name : module_info->mRequiredModules)
		{
			const auto* cached_module = sourceModule(project, name, err);
			if (cached_module == nullptr)
				return nullptr;

			// Add all cached child module dependencies
			module_deps.reserve(module_deps.size() + cached_module->mDependencies.size() + 1);
			for (const auto& child : cached_module->mDependencies)
				addDependency(module_deps, child);

			// Add direct module dependency
			if (!addDependency(module_deps, cached_module))
				nap::Logger::warn("Module '%s' is already included and can be removed as a requirement from '%s'",
					cached_module->getName().c_str(), moduleName.c_str());
		}

		// Load module binary
		std::string load_error;
		auto library_name = utility::appendFileExtension(moduleName, getModuleExtension());
		void* module_handle = loadModule(*module_info, library_name, module_info->mFilename, load_error);
		if (!module_handle)
		{
			err.fail("Failed to load module '%s' (resolved from %s): %s",
                moduleName.c_str(), utility::getAbsolutePath(module_json).c_str(), load_error.c_str());
			return nullptr;
		}

		// Find descriptor. If the descriptor wasn't found in the dll,
		// assume it's not actually a nap module and unload it again.
		auto descriptor = (ModuleDescriptor*)findSymbolInModule(module_handle, nap::getModuleDescriptorSymbolName(moduleName).c_str());
		if (!descriptor)
			return nullptr;

		// Verify module version
		if (descriptor->mAPIVersion != nap::moduleAPIVersion)
		{
			err.fail("Module %s version mismatch (found %d, expected %d)",
				moduleName.c_str(), descriptor->mAPIVersion, nap::moduleAPIVersion);
			return nullptr;
		}

		// Try to load service if one is defined
		rtti::TypeInfo service = rtti::TypeInfo::empty();
		if (descriptor->mService)
		{
			rtti::TypeInfo stype = rtti::TypeInfo::get_by_name(rttr::string_view(descriptor->mService));
			if (!stype.is_derived_from(RTTI_OF(Service)))
			{
				err.fail("Module '%s' service descriptor '%s' is not a service",
					moduleName.c_str(), descriptor->mService);
				return nullptr;
			}
			service = stype;
		}

		// Load successful, store loaded module
		auto module = std::make_unique<Module>();
		module->mName = moduleName;
		module->mDescriptor = descriptor;
		module->mInfo = std::move(module_info);
		module->mHandle = module_handle;
		module->mService = service;
		module->mDependencies = module_deps;

		nap::Logger::debug("Module loaded: %s", module->mDescriptor->mID);
		auto& it = mModules.emplace_back(std::move(module));
		return it.get();
	}
}
