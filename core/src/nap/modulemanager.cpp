/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "modulemanager.h"
#include "logger.h"
#include "service.h"
#include "module.h"
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	ModuleManager::ModuleManager(nap::Core& core) :
		mCore(core)
	{
		initModules();
	}


	bool ModuleManager::loadModules(const ProjectInfo& projectInfo, utility::ErrorState& err)
	{
		for (const std::string& moduleName : projectInfo.mRequiredModules)
			if (!sourceModule(projectInfo, moduleName, err))
				return false;
		return true;
	}


	std::vector<nap::Module*> ModuleManager::getModules() const
	{
		std::vector<nap::Module*> mods;
		mods.reserve(mModules.size());
		for (const auto& m : mModules)
			mods.emplace_back(m.get());
		return mods;
	}


	const Module* ModuleManager::findModule(const std::string& moduleName) const
	{
		auto found_it = std::find_if(mModules.begin(), mModules.end(), [&](const auto& it) {
			return it->mName == moduleName;
		});
		return found_it == mModules.end() ? nullptr : (*found_it).get();
	}


	const Module* ModuleManager::findModule(const nap::rtti::TypeInfo& serviceType) const
	{
		auto found_it = std::find_if(mModules.begin(), mModules.end(), [&](const auto& it) {
			return it->getServiceType() == serviceType;
			});
		return found_it == mModules.end() ? nullptr : (*found_it).get();
	}


	ModuleManager::~ModuleManager()
	{
		/*  Commented out for now because unloading modules can cause crashes in RTTR; during shutdown it will try
			to access RTTI types registered by potentially unloaded modules and crash because the pointers are no longer valid.
			
			This should probably be fixed in RTTR itself, but I'm not sure how yet. For now I've disabled the unloading of modules, 
			since this only happens during shutdown and modules will be unloaded then anyway.
		
		for (Module& module : mRequiredModules)
			UnloadModule(module.mHandle);
		*/
	}


	std::string Module::findAsset(const std::string& name) const
	{
		return utility::findFileInDirectories(name, this->getInformation().mDataSearchPaths);
	}
}
