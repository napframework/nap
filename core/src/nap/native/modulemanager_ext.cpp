/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/modulemanager.h>
#include <nap/logger.h>
#include <nap/module.h>
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
		if (!err.check(!moduleJson.empty(), "File '%s' not found in any of these dirs:\n%s",
			expectedJsonFile.c_str(), utility::joinString(moduleDirs, "\n").c_str()))
			return false;

		// Find module library in given directories
		auto expectedFilename = utility::stringFormat("%s.%s", moduleName.c_str(), getModuleExtension().c_str());
		moduleFile = utility::findFileInDirectories(expectedFilename, moduleDirs);
		if (!err.check(!moduleFile.empty(), "File '%s' not found in any of these dirs:\n%s",
			expectedFilename.c_str(), utility::joinString(moduleDirs, "\n").c_str()))
			return false;

		return true;
	}


	bool ModuleManager::sourceModule(const ProjectInfo& project, const std::string& moduleName, utility::ErrorState& err)
	{
		// Only load module once
		if (findModule(moduleName) != nullptr)
			return true;

		// Attempt to find the module files first
		std::string moduleFile;
		std::string moduleJson;
		if (!findModuleFiles(project, moduleName, moduleFile, moduleJson, err))
			return false;

		// Load module json
		auto modinfo = rtti::getObjectFromJSONFile<nap::ModuleInfo>(
			moduleJson,
			rtti::EPropertyValidationMode::AllowMissingProperties,
			mCore.getResourceManager()->getFactory(),
			err);

		if (!err.check(modinfo != nullptr,
					   "Failed to read %s from %s",
					   RTTI_OF(nap::ModuleInfo).get_name().data(),
					   moduleJson.c_str()))
			return false;

		// Store useful references so we can backtrack if necessary
		modinfo->mFilename = moduleFile;
		modinfo->mProjectInfo = &project;

		// Add project directory default search path for modules, used by Windows packaged apps
		modinfo->mLibSearchPaths.insert(modinfo->mLibSearchPaths.begin(), project.getProjectDir());

		// Patch template variables
		project.patchPaths(modinfo->mLibSearchPaths, {{"MODULE_DIR", utility::getFileDir(moduleFile)}});

		// Load module dependencies first
		for (const auto& modName : modinfo->mRequiredModules)
		{
			// Skip already loaded modules
			if (findModule(modName))
				continue;

			if (!sourceModule(project, modName, err))
				return false;
		}

		// Load module binary
		std::string loadModuleError;
		void* module_handle = loadModule(*modinfo, moduleFile, loadModuleError);
		if (!module_handle)
		{
			auto resolved = utility::getAbsolutePath(moduleFile);
			err.fail("Failed to load module '%s' (resolved as %s): %s",
				moduleFile.c_str(), resolved.c_str(), loadModuleError);
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

		// Verify module version
		if (descriptor->mAPIVersion != ModuleDescriptor::ModuleAPIVersion)
		{
			err.fail("Module %s version mismatch (found %d, expected %d)",
				moduleFile.c_str(), 
				descriptor->mAPIVersion,
				ModuleDescriptor::ModuleAPIVersion);
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
				err.fail("Module %s service descriptor %s is not a service", moduleFile.c_str(), descriptor->mService);
				unloadModule(module_handle);
				return false;
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
		nap::Logger::debug("Module loaded: %s", module->mDescriptor->mID);
		mModules.emplace_back(std::move(module));
		return true;
	}
}
