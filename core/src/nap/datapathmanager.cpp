#include "datapathmanager.h"
#include "logger.h"

#include <utility/fileutils.h>

namespace nap
{
	bool DataPathManager::populatePath(utility::ErrorState& errorState)
	{
		// Check if we have our data dir alongside our exe
		std::string testDataPath = utility::getExecutableDir() + "/data";
		if (utility::dirExists(testDataPath))
		{
			mDataPath = testDataPath;
			return true;
		}
		
		// Get project name
		// TODO do we have a better way to do get our project name?
		// TODO if not, once we have packaging work merged and we compile into project-specific paths maybe that folder name
		//      is cleaner?
		std::string projectName = utility::getFileNameWithoutExtension(utility::getExecutablePath());

		// Find NAP root.  Looks cludgey but we have control of this, it doesn't change.
		// TODO add another level to this then when merged with packaging and we have project-specific build paths
		std::string napRoot = utility::getAbsolutePath("../..");
	
		// Iterate possible project locations
		std::string possibleProjectParents[] = {
			"projects", // User projects against packaged NAP
			"examples", // Example projects in packaged NAP
			"demos", // Demo projects (maybe merge with examples?)
			"apps", // Applications in NAP source
			"test" // Old test projects in NAP source
		};
		for (auto& parentPath : possibleProjectParents)
		{
			testDataPath = napRoot + "/" + parentPath + "/" + projectName + "/data";
			if (utility::dirExists(testDataPath))
			{
				mDataPath = testDataPath;
				return true;
			}
		}

		errorState.fail("Couldn't find data for project %s", projectName.c_str());
		return false;
	}

	std::string DataPathManager::getDataPath()
	{
		return mDataPath;
	}
}
