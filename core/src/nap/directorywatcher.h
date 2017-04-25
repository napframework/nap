#pragma once

#include <string>
#include <memory>

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
		* @param modifiedFile: if the function returns true, contains the filename of the file that was modified. 
		*/
		bool update(std::string& modifiedFile);

	private:
#ifdef _WIN32
		struct PImpl;
		std::unique_ptr<PImpl> mPImpl = nullptr;
#endif
	};

} //< End Namespace nap

