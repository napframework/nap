// Local Includes
#include <nap/core.h>
#include <nap/logger.h>

// External Includes
#include <rtti/jsonreader.h>
#include <utility/fileutils.h>
#include <packaginginfo.h>

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
    

    bool Core::determineAndSetWorkingDirectory(utility::ErrorState& errorState, const std::string& forcedDataPath)
    {
        // If we've been provided with an explicit data path let's use that
        if (!forcedDataPath.empty())
        {
            // Verify path exists
            if (!utility::dirExists(forcedDataPath))
            {
                errorState.fail("Specified data path '%s' does not exist", forcedDataPath.c_str());
                return false;
            }
            else {
                utility::changeDir(forcedDataPath);
                return true;
            }
        }
        
        // Check if we have our data dir alongside our exe
        std::string testDataPath = utility::getExecutableDir() + "/data";
        if (utility::dirExists(testDataPath))
        {
            utility::changeDir(testDataPath);
            return true;
        }
        
        // Split up our executable path to scrape our project name
        std::string exeDir = utility::getExecutableDir();
        std::vector<std::string> dirParts;
        utility::splitString(exeDir, '/', dirParts);
        
        // Find NAP root.  Looks cludgey but we have control of this, it doesn't change.
        
        std::string napRoot;
        std::string projectName;
#ifdef NAP_PACKAGED_BUILD
        // We're running from a NAP release
        if (dirParts.size() >= 3)
        {
            // Non-packaged apps against released framework
            napRoot = utility::getAbsolutePath(exeDir + "/../../../../");
            projectName = dirParts.end()[-3];
        }
        else {
            errorState.fail("Unexpected path configuration found, could not locate project data");
            return false;
        }
#else // NAP_PACKAGED_BUILD
        // We're running from NAP source
        napRoot = utility::getAbsolutePath(exeDir + "/../../");
        projectName = utility::getFileNameWithoutExtension(utility::getExecutablePath());
#endif // NAP_PACKAGED_BUILD

        for (auto& parentPath : sPossibleProjectParents)
        {
            testDataPath = napRoot + "/" + parentPath + "/" + projectName + "/data";
            if (utility::dirExists(testDataPath))
            {
                utility::changeDir(testDataPath);
                return true;
            }
        }
        
        errorState.fail("Couldn't find data for project %s", projectName.c_str());
        return false;
    }


    bool Core::findProjectFilePath(const std::string& filename, std::string& foundFilePath) const
    {
        const std::string exeDir = utility::getExecutableDir();
        
        // Check for the file in its normal location, beside the binary
        const std::string alongsideBinaryPath = utility::getExecutableDir() + "/" + filename;
        nap::Logger::debug("Looking for '%s'...", alongsideBinaryPath.c_str());
        if (utility::fileExists(alongsideBinaryPath))
        {
            foundFilePath = alongsideBinaryPath;
            return true;
        }
#ifndef NAP_PACKAGED_BUILD
        // When working against NAP source find our file in the tree structure in the project source.
        // This is effectively a workaround for wanting to keep all binaries in the same root folder on Windows
        // so that we avoid module DLL copying hell.

        const std::string napRoot = utility::getAbsolutePath(exeDir + "/../../");
        const std::string projectName = utility::getFileNameWithoutExtension(utility::getExecutablePath());

        // Iterate possible project locations
        for (auto& parentPath : sPossibleProjectParents)
        {
            std::string testDataPath = napRoot + "/" + parentPath + "/" + projectName;
            nap::Logger::debug("Looking for project.json in '%s'...", testDataPath.c_str());
            if (!utility::dirExists(testDataPath))
                continue;

            // We found our project folder, now let's verify we have a our file in there
            testDataPath += "/";
            testDataPath += filename;
            nap::Logger::debug("Looking for '%s'...", testDataPath.c_str());
            if (utility::fileExists(testDataPath))
            {
                foundFilePath = testDataPath;
                return true;
            }
        }
#endif // NAP_PACKAGED_BUILD
        return false;
    }    


    bool Core::hasServiceConfiguration()
    {
        std::string config_file_path;   
        return findProjectFilePath(SERVICE_CONFIG_FILENAME, config_file_path);
    }


    bool Core::loadServiceConfiguration(rtti::DeserializeResult& deserializeResult, utility::ErrorState& errorState)
    {
        std::string config_file_path;        
        if (findProjectFilePath(SERVICE_CONFIG_FILENAME, config_file_path))
        {
            if (rtti::readJSONFile(config_file_path, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, mResourceManager->getFactory(), deserializeResult, errorState))
                return true;
        }

        return false;
    }
}