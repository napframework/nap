#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <fstream>
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <nap/core.h>
#include <entity.h>

using namespace nap;
using namespace nap::rtti;
using namespace nap::utility;

bool resolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers)
{
    std::map<std::string, RTTIObject*> objects_by_id;
    for (auto& object : objects)
        objects_by_id.insert({object->mID, object.get()});

    for (const UnresolvedPointer& unresolvedPointer : unresolvedPointers) {
        ResolvedRTTIPath resolved_path;
        if (!unresolvedPointer.mRTTIPath.resolve(unresolvedPointer.mObject, resolved_path))
            return false;

        auto pos = objects_by_id.find(unresolvedPointer.mTargetID);
        if (pos == objects_by_id.end())
            return false;

        if (!resolved_path.setValue(pos->second))
            return false;
    }

    return true;
}


void loadAndSaveFile(const std::string& filename)
{
    // Load File

    ErrorState err;

    Core core;
    if (!core.initializeEngine(err)) {
        nap::Logger::fatal(err.toString());
    }
    auto& factory = core.getResourceManager()->getFactory();
    nap::rtti::RTTIDeserializeResult result;
    if (!readJSONFile(filename, factory, result, err)) {
        nap::Logger::fatal(err.toString());
        return;
    }

    if (!resolveLinks(result.mReadObjects, result.mUnresolvedPointers)) {
        nap::Logger::fatal("Failed to resolve links");
        return;
    }

    // Save File
    ObjectList objects;
    for (auto& ob : result.mReadObjects) {
        objects.emplace_back(ob.get());
    }

    JSONWriter writer;
    if (!serializeObjects(objects, writer, err)) {
        nap::Logger::fatal(err.toString());
        return;
    }

    std::ofstream out(filename);
    out << writer.GetJSON();
    out.close();
}

int main(int argc, char** argv)
{
    std::string filename ="./data/tommy/tommy.json";

    loadAndSaveFile(filename);

}
