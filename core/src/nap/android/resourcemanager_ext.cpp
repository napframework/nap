// Local Includes
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
        // TODO ANDROID Cleanup, harden and code re-use. I believe this also doesn't cater for files over 1MB.

        // Open the asset using Android's AssetManager
        AAsset* asset = AAssetManager_open(mCore.getAndroidAssetManager(), filename.c_str(), AASSET_MODE_UNKNOWN);
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
        if (!deserializeJSON(outBuffer, EPropertyValidationMode::DisallowMissingProperties, getFactory(), readResult, errorState)) 
        {
            Logger::error("Failed to deserialise");
            return false;            
        }

        return true;
    }

    // At the moment on Android we're only managing files within the APK assets, which don't change, so we 
    // aren't handling changed files
    void ResourceManager::checkForFileChanges() { }
}