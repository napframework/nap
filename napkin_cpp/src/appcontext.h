#pragma once

#include <QObject>

#include <nap/core.h>
#include <nap/entity.h>
#include <rtti/rttireader.h>

#define WIN_GEO "windowGeometry"
#define WIN_STATE "windowState"
#define LAST_OPENED_FILE "lastOpenedFile"
#define NAP_FILE_FILTER "NAP JSON File (*.nap.json, *.json)"


class AppContext : public QObject {
Q_OBJECT

public:
    static AppContext& get();
    AppContext(AppContext const&) = delete;
    void operator=(AppContext const&) = delete;

    ~AppContext() override {}

    nap::Core& core() { return mCore; }

    void loadFile(const QString& filename);
    void saveFile();
    void saveFileAs(const QString& filename);
    const QString lastOpenedFilename();
    const QString& currentFilename() { return mCurrentFilename; }

    nap::rtti::OwnedObjectList& objects() { return mObjects; }
    nap::rtti::RTTIObject* getObject(const std::string& name);

    nap::Entity* getParent(const nap::Entity& entity);
    nap::Entity* getOwner(const nap::Component& component);
    nap::Entity* createEntity(nap::Entity* parent = nullptr);
    nap::Component* addComponent(nap::Entity& entity, rttr::type type);

    void deleteObject(nap::rtti::RTTIObject& object);

signals:
    void fileOpened(const QString& filename);
    void fileSaved(const QString& filename);

    void selectionChanged();

    // All data could have changed.
    void dataChanged();
    void entityAdded(nap::Entity* newEntity, nap::Entity* parent=nullptr);
    void componentAdded(nap::Component& comp, nap::Entity& owner);
    void objectRemoved(nap::rtti::RTTIObject& object);
private:
    AppContext();
    std::string getUniqueName(const std::string& suggestedName);

    std::vector<std::unique_ptr<nap::rtti::RTTIObject>> mObjects;
    QString mCurrentFilename;
    nap::Core mCore;
};