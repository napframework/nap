#include "../directorywatcher.h"
#include "assert.h"

namespace nap
{
	/**
	* Internal data container to hide internals from the header.
	*/
#ifdef _WIN32
	struct DirectoryWatcher::PImpl
	{
	};
#endif

	/**
	* Installs monitor: opens directory, creates event, starts directory scan.
	*/
	DirectoryWatcher::DirectoryWatcher()
	{
	}


	/**
	* Cleanup
	*/
	DirectoryWatcher::~DirectoryWatcher()
	{
	}


	/**
	* Checks if any changes to files were made, returns true if so. Continue to call this function to retrieve
	* multiple updates.
	*/
	bool DirectoryWatcher::update(std::vector<std::string>& modifiedFiles)
	{
		return false;
	}
}
