// Local Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <packaginginfo.h>

// External Includes
#include <rtti/jsonreader.h>
#include <utility/fileutils.h>

namespace nap
{
	static const std::string sPossibleProjectParents[] =
			{
					"projects",     // User projects against packaged NAP
					"examples",     // Example projects
					"demos",        // Demo projects
					"apps",         // Applications in NAP source
					"test"          // Old test projects in NAP source
			};


	bool Core::findProjectFilePath(const std::string& filename, std::string& foundFilePath) const
	{
		const std::string exeDir = utility::getExecutableDir();

		// Check for the file in its normal location, beside the binary
		const std::string alongsideBinaryPath = utility::joinPath({utility::getExecutableDir(), filename});
		nap::Logger::debug("Looking for '%s'...", alongsideBinaryPath.c_str());
		if (utility::fileExists(alongsideBinaryPath))
		{
			foundFilePath = alongsideBinaryPath;
			return true;
		}
		// When working against NAP Source or Packaged Release find our file in the tree structure in the project source.
		// This is effectively a workaround for wanting to keep all binaries in the same root folder on Windows
		// so that we avoid module DLL copying hell.

        // TODO This should use the path mapping
#ifdef NAP_PACKAGED_BUILD
		const std::string relNapRoot = utility::joinPath({exeDir, "..", "..", "..", ".."});
#else
		const std::string relNapRoot = utility::joinPath({exeDir, "..", ".."});
#endif
		const std::string napRoot = utility::getAbsolutePath(relNapRoot);
		const std::string projectName = utility::getFileNameWithoutExtension(utility::getExecutablePath());

		// Iterate possible project locations
		for (auto& parentPath : sPossibleProjectParents)
		{
			std::string testDataPath = utility::joinPath({napRoot, parentPath, projectName});
			nap::Logger::debug("Looking for %s in '%s'...", filename.c_str(), testDataPath.c_str());
			if (!utility::dirExists(testDataPath))
				continue;

			// We found our project folder, now let's verify we have a our file in there
			testDataPath = utility::joinPath({testDataPath, filename});
			if (utility::fileExists(testDataPath))
			{
				foundFilePath = testDataPath;
				nap::Logger::debug("Found '%s'...", foundFilePath.c_str());
				return true;
			}
		}
		return false;
	}


	bool Core::loadServiceConfiguration(const std::string& filename, rtti::DeserializeResult& deserializeResult, utility::ErrorState& errorState)
	{
		std::string config_file_path;
		if (findProjectFilePath(filename, config_file_path))
		{
			if (rtti::readJSONFile(config_file_path,
								   rtti::EPropertyValidationMode::DisallowMissingProperties,
								   rtti::EPointerPropertyMode::NoRawPointers,
								   mResourceManager->getFactory(),
								   deserializeResult,
								   errorState))
				return true;
		}

		return false;
	}
}
