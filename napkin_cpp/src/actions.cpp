#include "actions.h"

#include <QApplication>


OpenFileAction::OpenFileAction(QObject* parent) : QAction("Open...", parent) {
    setShortcut(QKeySequence::Open);
    connect(this, &QAction::triggered, this, &OpenFileAction::perform);
}

void OpenFileAction::perform() {
    auto lastFilename = AppContext::instance().lastOpenedFilename();
    QString filename = QFileDialog::getOpenFileName(QApplication::topLevelWidgets()[0],
                                                    "Open NAP Data File",
                                                    lastFilename,
                                                    NAP_FILE_FILTER);
    if (filename.isNull())
        return;

    AppContext::instance().openFile(filename);
}

SaveFileAction::SaveFileAction(QObject* parent) : QAction("Save", parent) {
    setShortcut(QKeySequence::Save);
    connect(this, &QAction::triggered, this, &SaveFileAction::perform);
}

void SaveFileAction::perform() {

}

SaveFileAsAction::SaveFileAsAction(QObject* parent) : QAction("Save as...", parent) {
    setShortcut(QKeySequence::SaveAs);
    connect(this, &QAction::triggered, this, &SaveFileAsAction::perform);
}

void SaveFileAsAction::perform() {

}
