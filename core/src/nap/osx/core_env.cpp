/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>
#include <stdlib.h>
#include <nap/logger.h>

namespace nap
{
    constexpr const char* moltenvk_icd = "MoltenVK_icd.json";

    void Core::setupPlatformSpecificEnvironment()
    {
        // Build Vulkan ICD path environment variable. We're currently blatting this blindly, which
        // won't do any harm, even if the project isn't using graphics.
        std::vector<std::string> paths({
            nap::utility::stringFormat("{ROOT}/system_modules/naprender/thirdparty/vulkansdk/macos/x86_64/share/vulkan/icd.d/%s", moltenvk_icd),
            nap::utility::stringFormat("{ROOT}/lib/%s", moltenvk_icd)
        });
        
        // Path the paths
        mProjectInfo->patchPaths(paths);

        // Set environment variable
        std::string icd_paths = utility::joinString(paths, ":");
        setenv("VK_ICD_FILENAMES", icd_paths.c_str(), 1);
    }
}
