#include "appcontext.h"
#include <QSettings>
#include <nap/resourcemanager.h>
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <fstream>

using namespace nap::rtti;
using namespace nap::utility;

bool ResolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers)
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

AppContext::AppContext()
{
    mCore.initialize();
}


AppContext& AppContext::get()
{
    static AppContext inst;
    return inst;
}

void AppContext::loadFile(const QString& filename)
{
    mCurrentFilename = filename;
    QSettings settings;
    settings.setValue(LAST_OPENED_FILE, filename);

    auto& factory = core().getOrCreateService<nap::ResourceManagerService>()->getFactory();
    ErrorState err;
    nap::rtti::RTTIDeserializeResult result;
    if (!readJSONFile(filename.toStdString(), factory, result, err)) {
        nap::Logger::fatal(err.toString());
        return;
    }

    if (!ResolveLinks(result.mReadObjects, result.mUnresolvedPointers)) {
        nap::Logger::fatal("Failed to resolve links");
        return;
    }

    // transfer
    mObjects.clear();
    for (auto& ob : result.mReadObjects) {
        mObjects.emplace_back(std::move(ob));
    }

    fileOpened(mCurrentFilename);
}

void AppContext::saveFile()
{
    saveFileAs(mCurrentFilename);
}

void AppContext::saveFileAs(const QString& filename)
{
    ObjectList objects;
    for (auto& ob : mObjects) {
        objects.emplace_back(ob.get());
    }

    JSONWriter writer;
    ErrorState err;
    if (!serializeObjects(objects, writer, err)) {
        nap::Logger::fatal(err.toString());
        return;
    }

    std::ofstream out(filename.toStdString());
    out << writer.GetJSON();
    out.close();

    mCurrentFilename = filename;
    nap::Logger::info("Written file: " + filename.toStdString());

    fileSaved(mCurrentFilename);
}


const QString AppContext::lastOpenedFilename()
{
    QSettings settings;
    return settings.value(LAST_OPENED_FILE).toString();
}

QList<rttr::instance> AppContext::selectedObjects() const
{

}




