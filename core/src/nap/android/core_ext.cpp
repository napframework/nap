// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/android/androidextension.h>
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


	bool Core::loadServiceConfiguration(const std::string& filename, rtti::DeserializeResult& deserializeResult, utility::ErrorState& errorState)
    {
		// Get interface
		const AndroidExtension& android_ext = getExtension<AndroidExtension>();

        // Open the asset using Android's AssetManager
        // TODO ANDROID Cleanup, harden and code re-use
        AAsset* asset = AAssetManager_open(android_ext.getAssetManager(), filename, AASSET_MODE_BUFFER);
        if (asset == NULL)
        {
            errorState.fail("AssetManager couldn't load %s", filename);
            return false;
        }

        // Read the asset
        long size = AAsset_getLength(asset);
        char* buffer = (char*) malloc (sizeof(char) * size);
        AAsset_read(asset, buffer, size);
        std::string outBuffer(buffer, size);
        AAsset_close(asset);

        if (!rtti::deserializeJSON(outBuffer, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, mResourceManager->getFactory(), deserializeResult, errorState))
            return false;

        return true;
    }
}
