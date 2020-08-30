#pragma once

#include <string>
#include <memory>
#include <vector>

// Local Includes
#include <utility/dllexport.h>
#include <utility/errorstate.h>

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
		 * Default constructor, make sure to call init() using the appropiate directory afterwards
		 */
		DirectoryWatcher() = default;

		// Destructor
		~DirectoryWatcher();
	
		// Copy is not allowed
		DirectoryWatcher(DirectoryWatcher&) = delete;

		// Copy assignment is not allowed
		DirectoryWatcher& operator=(const DirectoryWatcher&) = delete;

		// Move is not allowed
		DirectoryWatcher(DirectoryWatcher&&) = delete;

		// Move assignment is not allowed
		DirectoryWatcher& operator=(DirectoryWatcher&&) = delete;

		/**
		 * Checks if any changes to files were made, returns true if so.
		 * Continue to call this function to retrieve
		 * multiple updates.
		 * @param modifiedFiles: if the function returns true, contains the filenames of the files that were modified.
		 */
		bool update(std::vector<std::string>& modifiedFiles);

		/**
		 * Initializes the directory watcher using the provided directory.
		 * Needs to be implemented for each platform specifically.
		 * Note that the directory must exist!
		 * @param directory the directory to monitor.
		 */
		void init(const std::string& directory);

	private:
        struct PImpl;
        struct PImpl_deleter { void operator()(PImpl*) const; };
        std::unique_ptr<PImpl, PImpl_deleter> mPImpl = nullptr;
	};

} //< End Namespace nap

