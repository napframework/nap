/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

        // These three in order represent the three contexts: Source, Framework Release and 
        // Packaged App
        std::vector<std::string> paths({
            "{ROOT}/../thirdparty/vulkansdk/macos/x86_64/share/vulkan/icd.d/MoltenVK_icd.json",
            "{ROOT}/thirdparty/vulkansdk/share/vulkan/icd.d/MoltenVK_icd.json",
            "{EXE_DIR}/lib/MoltenVK_icd.json"
        });
        mProjectInfo->patchPaths(paths);
        
        std::string icd_paths = utility::joinString(paths, ":");
        setenv("VK_ICD_FILENAMES", icd_paths.c_str(), 1);
    }
}
