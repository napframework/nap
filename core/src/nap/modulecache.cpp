/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "modulecache.h"

// External includes
#include <utility/fileutils.h>

namespace nap
{
	nap::ModuleCache& ModuleCache::get()
	{
		static ModuleCache cache;
		return cache;
	}


	std::string Module::findAsset(const std::string& name) const
	{
		return utility::findFileInDirectories(name, this->getInformation().mDataSearchPaths);
	}
}
