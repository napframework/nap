#pragma once

#include <string>
#include <memory>
#include <vector>

namespace nap
{
	/**
	* Monitors file changes in a directory (currently hardcoded to active directory, and hardcoded to respond to file 'writes').
	* This class works polling-based, so continue to call update to retrieve what files are modified on disk.
	*/
	class DirectoryWatcher
	{
	public:
		DirectoryWatcher();
		~DirectoryWatcher();

		/**
		* Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve 
		* multiple updates.
		* @param modifiedFiles: if the function returns true, contains the filenames of the files that were modified. 
		*/
		bool update(std::vector<std::string>& modifiedFiles);

	private:
		struct PImpl;
		std::unique_ptr<PImpl> mPImpl = nullptr;
	};

} //< End Namespace nap

