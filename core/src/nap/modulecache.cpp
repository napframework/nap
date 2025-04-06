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
	//////////////////////////////////////////////////////////////////////////
	// Module
	//////////////////////////////////////////////////////////////////////////

	std::string Module::findAsset(const std::string& name) const
	{
		return utility::findFileInDirectories(name, this->getInformation().mDataSearchPaths);
	}


	//////////////////////////////////////////////////////////////////////////
	// Module Cache
	//////////////////////////////////////////////////////////////////////////

	ModuleCache::Handle::Handle(nap::ModuleCache& cache, std::mutex& mutex) : mLock(mutex),
		mCache(&cache)
	{ }


	ModuleCache::Handle::Handle(Handle&& other) noexcept
	{
		mLock = std::move(other.mLock);
		mCache = std::move(other.mCache);
	}


	nap::ModuleCache::Handle& ModuleCache::Handle::operator=(Handle&& other) noexcept
	{
		mLock = std::move(other.mLock);
		mCache = std::move(other.mCache);
		return *this;
	}


	nap::ModuleCache::Handle ModuleCache::getHandle()
	{
		// Create unique cache, local to unit
		static ModuleCache cache;
		static std::mutex mutex;
		static std::once_flag init_flag;

		// Platform specific initialization
		std::call_once(init_flag, [&]() {
				initModules();
			});

		// Create cache handle, lock and return
		return ModuleCache::Handle(cache, mutex);
	}


	const nap::Module* ModuleCache::findModule(const std::string& moduleName) const
	{
		auto found_it = std::find_if(mModules.begin(), mModules.end(), [&](const auto& it) {
			return it->mName == moduleName;
			});
		return found_it == mModules.end() ? nullptr : found_it->get();
	}


	const nap::Module* ModuleCache::Handle::sourceModule(const ProjectInfo& projectinfo, const std::string& moduleName, utility::ErrorState& err)
	{
		assert(mCache != nullptr);
		return mCache->sourceModule(projectinfo, moduleName, err);
	}
}
