// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/android/androidinterface.h>
#include <rtti/jsonreader.h>
#include <android/asset_manager.h>

namespace nap
{
    bool Core::determineAndSetWorkingDirectory(utility::ErrorState& errorState, const std::string& forcedDataPath)
    {
        // Don't set a working directory (for now?) on Android
        return true;
    }


    bool Core::findProjectFilePath(const std::string& filename, std::string& foundFilePath) const
    {
        // File found file will have the same path as the search path within the Android AssetManager
        // TODO ANDROID Confirm file exists using AssetManager
        foundFilePath = filename;

        return true;
    }


    bool Core::hasServiceConfiguration()
    {
		// Get interface
		const AndroidInterface& android_interface = getInterface<AndroidInterface>();

        // TODO ANDROID This is fairly temporary, use the AssetManager file list to determine if the file exists
        AAsset* asset = AAssetManager_open(android_interface.getAssetManager(), SERVICE_CONFIG_FILENAME, AASSET_MODE_UNKNOWN);
        bool has_config = asset != NULL;
        if (asset != NULL)
            AAsset_close(asset);
        return has_config;
    }


    bool Core::loadServiceConfiguration(rtti::DeserializeResult& deserializeResult, utility::ErrorState& errorState)
    {
		// Get interface
		const AndroidInterface& android_interface = getInterface<AndroidInterface>();

        // Open the asset using Android's AssetManager
        // TODO ANDROID Cleanup, harden and code re-use. I believe this also doesn't cater for files over 1MB.
        AAsset* asset = AAssetManager_open(android_interface.getAssetManager(), SERVICE_CONFIG_FILENAME, AASSET_MODE_UNKNOWN);
        if (asset == NULL)
        {
            errorState.fail("AssetManager couldn't load %s", SERVICE_CONFIG_FILENAME);
            return false;
        }

        // Read the asset
        long size = AAsset_getLength(asset);
        char* buffer = (char*) malloc (sizeof(char) * size);
        AAsset_read(asset, buffer, size);
        std::string outBuffer(buffer, size);
        AAsset_close(asset);

        if (!rtti::deserializeJSON(outBuffer, rtti::EPropertyValidationMode::DisallowMissingProperties,  mResourceManager->getFactory(), deserializeResult, errorState))
            return false;

        return true;
    }
}