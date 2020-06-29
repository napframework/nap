// Local Includes
#include "nap/modulemanager.h"
#include "nap/logger.h"
#include "nap/service.h"
#include "nap/module.h"
#include "nap/core.h"
#include "rtti/jsonreader.h"

// External Includes
#include <utility/fileutils.h>

namespace nap
{

	bool ModuleManager::loadModules(const ProjectInfo& projectInfo, utility::ErrorState& err)
	{
		for (const std::string& moduleName : projectInfo.mModuleNames)
			if (!loadModule_(projectInfo, moduleName, err))
				return false;

		return true;
	}

	std::vector<nap::Module*> ModuleManager::getModules() const
	{
		std::vector<nap::Module*> mods;
		for (const auto& m : mModules)
			mods.emplace_back(m.get());
		return mods;
	}

	bool ModuleManager::loadModule_(const ProjectInfo& project, const std::string& moduleName, utility::ErrorState& err)
	{
		// Only load module once
		if (getLoadedModule(moduleName))
			return true;

		// Attempt to find the module files first
		std::string moduleFile;
		std::string moduleJson;
		if (!findModuleFiles(project, moduleName, moduleFile, moduleJson, err))
			return false;

		// Load module json
		auto modinfo = rtti::readJSONFileObjectT<nap::ModuleInfo>(
			moduleJson,
			rtti::EPropertyValidationMode::AllowMissingProperties,
			rtti::EPointerPropertyMode::OnlyRawPointers,
			mCore.getResourceManager()->getFactory(),
			err);

		if (!err.check(modinfo != nullptr,
					   "Failed to read %s from %s",
					   RTTI_OF(nap::ModuleInfo).get_name().data(),
					   moduleJson.c_str()))
			return false;

		// Store the path so we can find more files later on
		modinfo->mFilename = moduleFile;

		// Load module dependencies first
		for (const auto& modName : modinfo->mDependencies)
		{
			// Skip already loaded modules
			if (getLoadedModule(modName))
				continue;

			if (!loadModule_(project, modName, err))
				return false;
		}

		// Load module binary
		std::string loadModuleError;
		void* module_handle = loadModule(moduleFile, loadModuleError);
		if (!module_handle)
		{
			err.fail(utility::stringFormat("Failed to load module '%s': %s",
										   moduleFile.c_str(), loadModuleError.c_str()));
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

	const Module* ModuleManager::getLoadedModule(const std::string& moduleName)
	{
		for (const auto& mod : mModules)
			if (mod->mName == moduleName)
				return mod.get();
		return nullptr;
	}

	bool ModuleManager::findModuleFiles(const ProjectInfo& projectInfo, const std::string& moduleName,
										std::string& moduleFile, std::string& moduleJson, utility::ErrorState& err)
	{
		// Require module directories to be provided
		auto moduleDirs = projectInfo.getModuleDirectories();
		if (!err.check(!moduleDirs.empty(), "No module dirs specified in %s", projectInfo.mFilename.c_str()))
			return false;

        // Substitute module name in given directories
        std::string token = "{MODULE_NAME}";
		for (auto& dir : moduleDirs)
        {
            size_t found_pos = dir.find(token);
            if(found_pos == std::string::npos)
                continue;
            dir.replace(found_pos, token.length(), moduleName);
        }

		// Find module json in given directories
		auto expectedJsonFile = utility::stringFormat("%s.json", moduleName.c_str());
		moduleJson = utility::findFileInDirectories(expectedJsonFile, moduleDirs);
		if (!err.check(!moduleJson.empty(), "File '%s' not found in any of these dirs:\n%s",
					   expectedJsonFile.c_str(), utility::joinString(moduleDirs, "\n").c_str()))
			return false;

		// Find module library in given directories
		auto expectedFilename = utility::stringFormat("%s.%s", moduleName.c_str(), getModuleExtension().c_str());
		moduleFile = utility::findFileInDirectories(expectedFilename, moduleDirs);
		if (!err.check(!moduleJson.empty(), "File '%s' not found in any of these dirs:\n%s",
					   expectedFilename.c_str(), utility::joinString(moduleDirs, "\n").c_str()))
			return false;

		return true;
	}

}
