/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "modulecache.h"
#include "module.h"

// External includes
#include <utility/fileutils.h>

namespace nap
{
	// Local to unit
	static ModuleCache	sModuleCache;
	static std::mutex	sModuleCacheMutex;
	ModuleCache::Handle::Handle() : mLock(sModuleCacheMutex),
		mCache(&sModuleCache)
	{ }


	nap::ModuleCache::Handle ModuleCache::getHandle()
	{
		static std::once_flag module_cache_flag;
		std::call_once(module_cache_flag, [&]() {
				initModules();
			});
		return ModuleCache::Handle();
	}


	const nap::Module* ModuleCache::findModule(const std::string& moduleName) const
	{
		auto found_it = std::find_if(mModules.begin(), mModules.end(), [&](const auto& it) {
			return it->mName == moduleName;
			});
		return found_it == mModules.end() ? nullptr : found_it->get();
	}


	std::string Module::findAsset(const std::string& name) const
	{
		return utility::findFileInDirectories(name, this->getInformation().mDataSearchPaths);
	}


	const nap::Module* ModuleCache::Handle::sourceModule(const ProjectInfo& projectinfo, const std::string& moduleName, utility::ErrorState& err)
	{
		assert(mCache != nullptr);
		return mCache->sourceModule(projectinfo, moduleName, err);
	}
}
