// Local Includes
#include <nap/resourcemanager.h>

// External Includes
#include <rtti/jsonreader.h>

namespace nap
{
    using namespace rtti;

    bool ResourceManager::loadFileAndDeserialize(const std::string& filename, DeserializeResult& readResult, utility::ErrorState& errorState)
    {
        // Read objects from disk
        return readJSONFile(filename, EPropertyValidationMode::DisallowMissingProperties, getFactory(), readResult, errorState);
    }
}