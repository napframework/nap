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

    nap::rtti::OwnedObjectList& loadedObjects() { return mObjects; }

    nap::Entity* getParent(const nap::Entity& entity);
    nap::Entity* createEntity(nap::Entity* parent = nullptr);;

signals:
    void fileOpened(const QString& filename);
    void fileSaved(const QString& filename);

    void selectionChanged();

    // All data could have changed.
    void dataChanged();
    void entityAdded(nap::Entity* newEntity, nap::Entity* parent=nullptr);

private:
    AppContext();


    nap::rtti::OwnedObjectList mObjects;
    QString mCurrentFilename;
    nap::Core mCore;
};