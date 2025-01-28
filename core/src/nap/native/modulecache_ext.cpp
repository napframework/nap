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
	static bool findModuleFiles(const ProjectInfo& projectInfo, const std::string& moduleName,
		std::string& moduleFile, std::string& moduleJson, utility::ErrorState& err)
	{
		// Require module directories to be provided
		auto moduleDirs = projectInfo.getModuleDirectories();
		if (!err.check(!moduleDirs.empty(), "No module dirs specified in %s", projectInfo.getFilename().c_str()))
			return false;

		// Substitute module name in given directories
		utility::namedFormat(moduleDirs, { { "MODULE_NAME", moduleName } });

		// Find module json in given directories
		auto expectedJsonFile = utility::stringFormat("%s.json", moduleName.c_str());
		moduleJson = utility::findFileInDirectories(expectedJsonFile, moduleDirs);
		if (!err.check(!moduleJson.empty(), "Module descriptor '%s' not found in any of these dirs:\n%s",
			expectedJsonFile.c_str(), utility::joinString(moduleDirs, "\n").c_str()))
			return false;

		// Find module library in given directories
		auto expectedFilename = utility::stringFormat("%s.%s", moduleName.c_str(), getModuleExtension().c_str());
		moduleFile = utility::findFileInDirectories(expectedFilename, moduleDirs);
		if (!err.check(!moduleFile.empty(), "Module library '%s' not found in any of these dirs:\n%s",
			expectedFilename.c_str(), utility::joinString(moduleDirs, "\n").c_str()))
			return false;

		return true;
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

		// Attempt to find the module files first
		std::string moduleFile;
		std::string moduleJson;
		if (!findModuleFiles(project, moduleName, moduleFile, moduleJson, err))
			return nullptr;

		// Load module json
		nap::rtti::Factory factory;
		auto modinfo = rtti::getObjectFromJSONFile<nap::ModuleInfo>(
			moduleJson,
			rtti::EPropertyValidationMode::AllowMissingProperties,
			factory,
			err);

		if (!err.check(modinfo != nullptr, "Failed to read %s from %s",
			RTTI_OF(nap::ModuleInfo).get_name().data(),  moduleJson.c_str()))
			return false;

		// Store useful references so we can backtrack if necessary
		modinfo->mFilename = moduleFile;
		modinfo->mProjectInfo = &project;

		// Add project directory default search path for modules, used by Windows packaged apps
		modinfo->mLibSearchPaths.insert(modinfo->mLibSearchPaths.begin(), project.getProjectDir());

		// Patch library search paths
		std::string file_dir = utility::getFileDir(moduleFile);
		project.patchPaths(modinfo->mLibSearchPaths, {{"MODULE_DIR", file_dir}});

		// Patch library data paths
		project.patchPaths(modinfo->mDataSearchPaths,
			{
				{"MODULE_NAME", moduleName},
				{"MODULE_DIR", file_dir}
			});

		// Recursively load module dependencies first
		std::vector<const nap::Module*> module_deps;
		for (const auto& modName : modinfo->mRequiredModules)
		{
			const auto* cached_module = sourceModule(project, modName, err);
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
		std::string loadModuleError;
		void* module_handle = loadModule(*modinfo, moduleFile, loadModuleError);
		if (!module_handle)
		{
			auto resolved = utility::getAbsolutePath(moduleFile);
			err.fail("Failed to load module '%s' (resolved as %s): %s",
                utility::forceSeparator(moduleFile).c_str(), resolved.c_str(), loadModuleError.c_str());
			return nullptr;
		}

		// Find descriptor. If the descriptor wasn't found in the dll,
		// assume it's not actually a nap module and unload it again.
		auto descriptor = (ModuleDescriptor*)findSymbolInModule(module_handle, nap::getModuleDescriptorSymbolName(moduleName).c_str());
		if (!descriptor)
		{
			unloadModule(module_handle);
			return nullptr;
		}

		// Verify module version
		if (descriptor->mAPIVersion != nap::moduleAPIVersion)
		{
			err.fail("Module %s version mismatch (found %d, expected %d)",
				moduleFile.c_str(), 
				descriptor->mAPIVersion,
				nap::moduleAPIVersion);
			unloadModule(module_handle);
			return nullptr;
		}

		// Try to load service if one is defined
		rtti::TypeInfo service = rtti::TypeInfo::empty();
		if (descriptor->mService)
		{
			rtti::TypeInfo stype = rtti::TypeInfo::get_by_name(rttr::string_view(descriptor->mService));
			if (!stype.is_derived_from(RTTI_OF(Service)))
			{
				err.fail("Module %s service descriptor %s is not a service", moduleFile.c_str(), descriptor->mService);
				unloadModule(module_handle);
				return nullptr;
			}
			service = stype;
		}

		// Load successful, store loaded module
		auto module = std::make_unique<Module>();
		module->mName = moduleName;
		module->mDescriptor = descriptor;
		module->mInfo = std::move(modinfo);
		module->mHandle = module_handle;
		module->mService = service;
		module->mDependencies = module_deps;

		nap::Logger::debug("Module loaded: %s", module->mDescriptor->mID);
		auto& it = mModules.emplace_back(std::move(module));
		return it.get();
	}
}

