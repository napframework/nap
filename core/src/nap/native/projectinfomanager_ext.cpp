// Local Includes
#include <nap/projectinfomanager.h>

// External Includes
#include <nap/core.h>
#include <utility/fileutils.h>

#include <fstream>

namespace nap
{   
    bool loadProjectInfoFromFile(const Core& core, ProjectInfo& result, utility::ErrorState& errorState) 
    {
        std::string projectFile;
        if (!errorState.check(core.findProjectFilePath(PROJECT_INFO_FILENAME, projectFile), "Couldn't find project.json beside binary or in project folder"))
            return false;

        bool loaded = false;

        // Open the file
        std::ifstream in(projectFile, std::ios::in | std::ios::binary);
        loaded = loadProjectInfoFromStream(core, result, errorState, in);
        in.close();

        return errorState.check(loaded, "Unable to open file %s", utility::getAbsolutePath(projectFile).c_str());
    }
}