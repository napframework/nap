#include "sequenceeditorguiclipboard.h"
#include "utility/fileutils.h"

#include <sequencetracksegment.h>
#include <nap/logger.h>
#include <rtti/jsonwriter.h>

namespace nap
{
    using namespace sequenceguiclipboard;


    Clipboard::Clipboard(const rttr::type &trackType)
        : mTrackType(trackType)
    {
    }


    void Clipboard::addObject(const rtti::Object *object, const std::string &sequenceName, utility::ErrorState &errorState)
    {
        // clear serialized objects if we loaded another show and add a segment from that sequence
        if(sequenceName != mSequenceName)
        {
            mSerializedObjects.clear();
        }
        mSequenceName = sequenceName;

        addObject(object, errorState);
    }


    void Clipboard::addObject(const rtti::Object *object, utility::ErrorState &errorState)
    {
        // first remove the object if it already exists in the clipboard
        if(containsObject(object->mID, mSequenceName))
        {
            removeObject(object->mID);
        }

        /*
         * TODO : FIX CONST CAST
         * I'm forced to do a const cast because the serializeObjects call does not accept a list of const object pointers
         * This should however be possible in my view, since serialization only requires reading from the object and never writing
         * So either we copy the object, making in non-const OR we make the serializeObjects call work with const object pointers
         */

        // serialize the object
        rtti::JSONWriter writer;
        if(!rtti::serializeObjects(rtti::ObjectList{const_cast<rtti::Object *>(object)}, writer, errorState))
        {
            nap::Logger::error("Error serializing object %s , error : ", object->mID.c_str(), errorState.toString().c_str());
        } else
        {
            std::string serialized_object = writer.GetJSON();
            mSerializedObjects.insert(std::pair<std::string, std::string>(object->mID, serialized_object));
        }
    }


    bool Clipboard::containsObject(const std::string &objectID, const std::string &sequenceName) const
    {
        // different sequence so, does not contain
        if(sequenceName != mSequenceName)
            return false;

        return mSerializedObjects.find(objectID) != mSerializedObjects.end();
    }


    void Clipboard::removeObject(const std::string &objectID)
    {
        auto it = mSerializedObjects.find(objectID);
        if(it != mSerializedObjects.end())
        {
            mSerializedObjects.erase(it);
        }
    }


    std::vector<std::string> Clipboard::getObjectIDs() const
    {
        std::vector<std::string> ids;
        for(const auto &pair: mSerializedObjects)
        {
            ids.emplace_back(pair.first);
        }

        return ids;
    }


    bool Clipboard::save(const std::string &filePath, utility::ErrorState &errorState)
    {
        // Get file name and ensure it gets a '.json' extension
        std::string file_name = utility::getFileNameWithoutExtension(filePath);
        file_name = utility::appendFileExtension(file_name, "json");

        // Get file path, create directory if it doesn't exist
        std::string directory = utility::getFileDir(filePath);
        if(!directory.empty())
        {
            if(!errorState.check(utility::ensureDirExists(directory), "unable to write to directory: %s", directory.c_str()))
                return false;
        }

        // create final file path
        std::string file_path;
        if(!directory.empty())
        {
            file_path = utility::joinPath({directory, file_name});
        } else
        {
            file_path = file_name;
        }

        // Open output file
        std::ofstream output_stream(file_path, std::ios::binary | std::ios::out);
        if(!errorState.check(
            output_stream.is_open() && output_stream.good(), "Failed to open %s for writing", file_path.c_str()))
            return false;

        // Create a set of object ptrs to write to disk
        std::vector<std::unique_ptr<rtti::Object>> owned_objects;
        auto object_ptrs = deserialize<rtti::Object>(owned_objects, errorState);
        if(errorState.hasErrors())
            return false;

        // Serialize current set of parameters to json
        rtti::JSONWriter writer;
        if(!rtti::serializeObjects(object_ptrs, writer, errorState))
            return false;

        // Write to disk
        std::string json = writer.GetJSON();
        output_stream.write(json.data(), json.size());

        return true;
    }


    bool Clipboard::load(const std::string &filePath, utility::ErrorState &errorState)
    {
        rtti::DeserializeResult result;

        // Ensure file exists
        if(!errorState.check(!filePath.empty() && utility::fileExists(filePath), "Preset does not exist"))
            return false;

        //
        rtti::Factory factory;
        if(!rtti::deserializeJSONFile(filePath,
                                      rtti::EPropertyValidationMode::DisallowMissingProperties,
                                      rtti::EPointerPropertyMode::NoRawPointers,
                                      factory,
                                      result,
                                      errorState))
            return false;

        // Resolve links
        if(!rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, errorState))
            return false;

        mSerializedObjects.clear();
        for(auto &read_object: result.mReadObjects)
        {
            if(read_object->get_type().is_derived_from<SequenceTrackSegment>())
            {
                addObject(read_object.get(), errorState);
            }
        }

        if(errorState.hasErrors())
        {
            mSerializedObjects.clear();
            return false;
        }


        return true;
    }
}