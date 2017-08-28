#include "appcontext.h"
#include <QSettings>
#include <nap/resourcemanager.h>




void AppContext::openFile(const QString& filename) {
    mCurrentFilename = filename;
    QSettings settings;
    settings.setValue(LAST_OPENED_FILE, filename);

    auto resman = core().getOrCreateService<nap::ResourceManagerService>();
    if (!resman->loadFile(filename.toStdString())) {
        nap::Logger::warn("Not opening file");
        return;
    }

    fileOpened(mCurrentFilename);
}

const QString AppContext::lastOpenedFilename() {
    QSettings settings;
    return settings.value(LAST_OPENED_FILE).toString();
}

