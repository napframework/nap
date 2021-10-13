/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <string>
#include <memory>
#include <vector>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Monitors file changes in a directory and its subdirectories.
	 * (currently hardcoded to active directory (cwd), and hardcoded to respond to file 'writes').
	 * This class works polling-based, so continue to call update to retrieve what files are modified on disk.
	 */
	class NAPAPI DirectoryWatcher
	{
	public:
		/**
		 * @param directory the directory to watch
		 */
		DirectoryWatcher(const std::string& directory);
		~DirectoryWatcher();

		/**
		 * Checks if any changes to files were made, returns true if so.
		 * Continue to call this function to retrieve multiple updates.
		 * @param modifiedFiles the filenames of the files that were modified.
		 * @return if files have been modified 
		 */
		bool update(std::vector<std::string>& modifiedFiles);

	private:
        struct PImpl;
        struct PImpl_deleter { void operator()(PImpl*) const; };
        std::unique_ptr<PImpl, PImpl_deleter> mPImpl = nullptr;
	};

} //< End Namespace nap

