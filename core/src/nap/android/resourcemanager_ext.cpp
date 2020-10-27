/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <nap/android/androidextension.h>
#include <nap/resourcemanager.h>
#include <nap/core.h>
#include <nap/logger.h>

// External Includes
#include <rtti/jsonreader.h>
#include <android/asset_manager.h>

namespace nap
{
    using namespace rtti;

    bool ResourceManager::loadFileAndDeserialize(const std::string& filename, DeserializeResult& readResult, utility::ErrorState& errorState)
    {
		if (!errorState.check(mCore.hasExtension<AndroidExtension>(), "Core not setup with Android extension!"))
			return false;

		// Get interface
		const AndroidExtension& android_ext = mCore.getExtension<AndroidExtension>();

        // Open the asset using Android's AssetManager
        // TODO ANDROID Cleanup, harden and code re-use
        AAsset* asset = AAssetManager_open(android_ext.getAssetManager(), filename.c_str(), AASSET_MODE_BUFFER);
        if (asset == NULL) 
        {
            Logger::error("AssetManager couldn't load asset %s", filename.c_str());
            return false;
        }

        // Read the asset
        long size = AAsset_getLength(asset);
        char* buffer = (char*) malloc (sizeof(char) * size);
        AAsset_read(asset, buffer, size);
        std::string outBuffer(buffer, size);
        AAsset_close(asset);

        // Process the loaded JSON
        if (!deserializeJSON(outBuffer, EPropertyValidationMode::DisallowMissingProperties, EPointerPropertyMode::NoRawPointers, getFactory(), readResult, errorState)) 
        {
            Logger::error("Failed to de-serialize");
            return false;            
        }
        return true;
    }

    // At the moment on Android we're only managing files within the APK assets, which don't change, so we 
    // aren't handling changed files
    void ResourceManager::checkForFileChanges() { }
}