// Local Includes
#include <nap/resourcemanager.h>
#include <nap/logger.h>

// External Includes
#include <rtti/jsonreader.h>
#include <utility/fileutils.h>

namespace nap
{
    using namespace rtti;

    bool ResourceManager::loadFileAndDeserialize(const std::string& filename, DeserializeResult& readResult, utility::ErrorState& errorState)
    {
        // Read objects from disk
        return deserializeJSONFile(filename, EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, getFactory(), readResult, errorState);
    }


    void ResourceManager::checkForFileChanges()
    {
        std::vector<std::string> modified_files;
        if (mDirectoryWatcher != nullptr && mDirectoryWatcher->update(modified_files))
        {
            for (std::string& modified_file : modified_files)
            {
                // Multiple events for the same file may occur, and we do not want to reload for every event given.
                // Instead we check the filetime and store that filetime in an internal map. If an event comes by that
                // with a filetime that we already processed, we ignore it.
                // It may also be possible that events are thrown for files that we do not have access to, or for files
                // that have been removed in the meantime. For these cases, we ignore events where the filetime check
                // fails.
                EFileModified file_modified = isFileModified(modified_file);
                if (file_modified == EFileModified::Error || file_modified == EFileModified::No)
                    continue;

                modified_file = utility::toComparableFilename(modified_file);
                std::set<std::string> files_to_reload;

                // Is our modified file a json file that was loaded by the manager?
                if (mFilesToWatch.find(modified_file) != mFilesToWatch.end())
                {
                    files_to_reload.insert(modified_file);
                }
                else
                {
                    // Non-json file. Find all the json sources of this file
                    FileLinkMap::iterator file_link = mFileLinkMap.find(modified_file);
                    if (file_link != mFileLinkMap.end())
                        for (const std::string& source_file : file_link->second)
                            files_to_reload.insert(source_file);
                }

                if (!files_to_reload.empty())
                {
                    nap::Logger::info("Detected change to %s. Files needing reload:", modified_file.c_str());
                    for (const std::string& source_file : files_to_reload)
                        nap::Logger::info("\t-> %s", source_file.c_str());

                    for (const std::string& source_file : files_to_reload)
                    {
                        utility::ErrorState errorState;
                        if (!loadFile(source_file, source_file == modified_file ? std::string() : modified_file, errorState))
                        {
                            nap::Logger::warn("Failed to reload %s (%s)", source_file.c_str(), errorState.toString().c_str());
                            break;
                        }
                    }
                }
            }
        }
    }    
}