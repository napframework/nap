/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <packaginginfo.h>

// External Includes
#include <rtti/jsonreader.h>
#include <utility/fileutils.h>

namespace nap
{
	static const std::vector<std::string> sPossibleProjectParents =
	{
			"projects",     // User projects against packaged NAP
			"demos",        // Demo projects
			"apps",         // Applications in NAP source
			"test",         // Old test projects in NAP source
			"examples"      // Example projects
	};


	bool Core::findProjectInfoFile(std::string& foundFilePath) const
	{
		// Locate the app.json file without using the path mapping system as it hasn't been initialized yet.

		const std::string exeDir = utility::getExecutableDir();

		// Check for its Packaged App location, beside the binary
		const std::string alongsideBinaryPath = utility::joinPath({exeDir, PROJECT_INFO_FILENAME});
		nap::Logger::debug("Looking for %s in '%s'...", PROJECT_INFO_FILENAME, exeDir.c_str());
		if (utility::fileExists(alongsideBinaryPath))
		{
			foundFilePath = alongsideBinaryPath;
			return true;
		}

		// When working against NAP Source or Framework Release find our file in the tree structure in the project source.
		// This complexity results from having different build directory structures for Source and Framework Release 
		// contexts (.. which itself originally resulted from wanting to keep all the binaries in the same root folder on 
		// Windows to avoid module DLL copying hell). The intention is to in future bring the directory structures more
		// in line with each other, which will greatly simplify this search.

#ifdef NAP_PACKAGED_BUILD
		const std::string relNapRoot = utility::joinPath({exeDir, "..", "..", "..", ".."});
#else
		const std::string relNapRoot = utility::joinPath({exeDir, "..", ".."});
#endif
		const std::string napRoot = utility::getAbsolutePath(relNapRoot);
		const std::string projectName = utility::getFileNameWithoutExtension(utility::getExecutablePath());

		// Iterate possible project locations
		for (const auto& parentPath : sPossibleProjectParents)
		{
			std::string testPath = utility::joinPath({napRoot, parentPath, projectName});
			nap::Logger::debug("Looking for %s in '%s'...", PROJECT_INFO_FILENAME, testPath.c_str());
			if (!utility::dirExists(testPath))
				continue;

			// We found our project folder, now let's verify we have a our file in there
			testPath = utility::joinPath({testPath, PROJECT_INFO_FILENAME});
			if (utility::fileExists(testPath))
			{
				foundFilePath = testPath;
				nap::Logger::debug("Found '%s'...", foundFilePath.c_str());
				return true;
			}
		}
		return false;
	}


	bool Core::findProjectFilePath(const std::string& filename, std::string& foundFilePath) const
	{
		const std::string projectDir = mProjectInfo->getProjectDir();
		std::string testPath = utility::joinPath({projectDir, filename});
		nap::Logger::debug("Looking for %s in '%s'...", filename.c_str(), projectDir.c_str());
		if (utility::fileExists(testPath))
		{
			foundFilePath = testPath;
			nap::Logger::debug("Found '%s'...", foundFilePath.c_str());
			return true;
		}
		return false;
	}


	bool Core::loadServiceConfiguration(const std::string& filename, rtti::DeserializeResult& deserializeResult, utility::ErrorState& errorState)
	{
		// Get path relative to project
		std::string config_file_path;
		if (!errorState.check(findProjectFilePath(filename, config_file_path), "Unable to find service configuration file: %s", filename.c_str()))
			return false;

		// Load
		return rtti::deserializeJSONFile(config_file_path,
			rtti::EPropertyValidationMode::DisallowMissingProperties,
			rtti::EPointerPropertyMode::NoRawPointers,
			mResourceManager->getFactory(),
			deserializeResult,
			errorState);
	}
}
