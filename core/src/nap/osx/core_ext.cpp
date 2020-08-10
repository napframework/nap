// Local Includes
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>

#include <stdlib.h>

namespace nap
{
    void Core::setupPlatformSpecificEnvironment()
    {
        // Build Vulkan ICD path environment variable. We're currently blatting this blindly, which
        // won't do any harm, even if the project isn't using graphics.
        
        // TODO Once napkin_relocation has merged use path mapping instead to locate thirdparty
        
        // For Source context
        std::string exe_dir = utility::getExecutableDir();
        std::string icd_paths = exe_dir + "/../../../thirdparty/vulkansdk/osx/share/vulkan/icd.d/MoltenVK_icd.json";

        // For Framework Release context
        icd_paths.append(":" + exe_dir + "/../../../../thirdparty/vulkansdk/share/vulkan/icd.d/MoltenVK_icd.json");

        // For Packaged App context
        icd_paths.append(":" + exe_dir + "/lib/vulkansdk/share/vulkan/icd.d/MoltenVK_icd.json");
        setenv("VK_ICD_FILENAMES", icd_paths.c_str(), 1);
    }
}
