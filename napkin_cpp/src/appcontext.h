#pragma once

#include <QObject>


#include <nap/core.h>
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
    void loadFile(const QString& filename);
    void saveFile();
    void saveFileAs(const QString& filename);
    nap::Core& core() { return mCore; }
    const QString lastOpenedFilename();
    nap::rtti::OwnedObjectList& loadedObjects() { return mObjects; }
    const QString& currentFilename() { return mCurrentFilename; }

signals:

    void fileOpened(const QString& filename);
    void fileSaved(const QString& filename);

private:
    AppContext() = default;


    nap::rtti::OwnedObjectList mObjects;
    QString mCurrentFilename;
    nap::Core mCore;
};