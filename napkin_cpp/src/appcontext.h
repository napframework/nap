#pragma once

#include <QObject>


#include <nap/core.h>

#define WIN_GEO "windowGeometry"
#define WIN_STATE "windowState"
#define LAST_OPENED_FILE "lastOpenedFile"
#define NAP_FILE_FILTER "NAP JSON File (*.nap.json, *.json)"


class AppContext : public QObject {
Q_OBJECT

public:
    static AppContext& instance() {
        static AppContext inst;
        return inst;
    }

    virtual ~AppContext() {}

    void openFile(const QString& filename);

    nap::Core& core() { return mCore; }

    const QString lastOpenedFilename();

signals:

    void fileOpened(const QString& filename);

public:
    AppContext(AppContext const&) = delete;

    void operator=(AppContext const&) = delete;


private:
    AppContext() {}

    QString mCurrentFilename;
    nap::Core mCore;
};