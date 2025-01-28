/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "modulemanager.h"
#include "logger.h"
#include "service.h"
#include "module.h"

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>
#include <mutex>

namespace nap
{
	static bool includeModule(const nap::Module* sourced, std::vector<const Module*>& ioResult)
	{
		// Find sourced module in list
		auto loaded = std::find_if(ioResult.begin(), ioResult.end(), [&sourced](const auto& it) {
			return it == sourced;
		});

		// Add when not included
		if (loaded == ioResult.end())
		{
			ioResult.emplace_back(sourced);
			return true;
		}
		return false;
	}


	bool ModuleManager::loadModules(const ProjectInfo& projectInfo, utility::ErrorState& err)
	{
		std::vector<const nap::Module*> sourced_modules;
		{
			ModuleCache::Handle cache = ModuleCache::getHandle();
			for (const std::string& moduleName : projectInfo.mRequiredModules)
			{
				const auto* module = cache.sourceModule(projectInfo, moduleName, err);
				if (module == nullptr)
					return false;

				for (const auto& child : module->getDependencies())
					includeModule(child, sourced_modules);

				if(!includeModule(module, sourced_modules))
					nap::Logger::warn("Module '%s' is already included and can be removed as a requirement from '%s'",
						module->getName().c_str(), projectInfo.mTitle.c_str());
			}
		}

		// Move modules
		mModules = std::move(sourced_modules);
		return true;
	}


	const Module* ModuleManager::findModule(const std::string& moduleName) const
	{
		auto found_it = std::find_if(mModules.begin(), mModules.end(), [&](const auto& it) {
			return it->getName() == moduleName;
		});
		return found_it == mModules.end() ? nullptr : *found_it;
	}


	const Module* ModuleManager::findModule(const nap::rtti::TypeInfo& serviceType) const
	{
		auto found_it = std::find_if(mModules.begin(), mModules.end(), [&](const auto& it) {
			return it->getServiceType() == serviceType;
			});
		return found_it == mModules.end() ? nullptr : *found_it;
	}
}

