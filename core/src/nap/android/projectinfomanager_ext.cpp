/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/android/androidextension.h>
#include <nap/projectinfomanager.h>
#include <nap/core.h>
#include <android/asset_manager.h>

namespace nap
{   
    bool loadProjectInfoFromFile(const Core& core, ProjectInfo& result, utility::ErrorState& errorState) 
    {
        bool loaded = false;

        // TODO ANDROID Check if file exists before attempting to load, allowing us to report on missing project.json
		if (!errorState.check(core.hasExtension<AndroidExtension>(), "Core not setup with Android extension!"))
			return false;
		
		// Get interface
		const AndroidExtension& android_ext = core.getExtension<AndroidExtension>();
        // TODO ANDROID Temporary project info loading, needs error handling, code re-use, etc
        AAsset* asset = AAssetManager_open(android_ext.getAssetManager(), PROJECT_INFO_FILENAME, AASSET_MODE_BUFFER);
        if (asset != NULL) 
        {
            long size = AAsset_getLength(asset);
            char* buffer = (char*) malloc (sizeof(char) * size);
            AAsset_read (asset, buffer, size);
            loaded = deserializeProjectInfoJSON(std::string(buffer, size), result, errorState);
            AAsset_close(asset);
        }       

        return errorState.check(loaded, "Unable to open file %s using AssetManager", PROJECT_INFO_FILENAME);
    }
}