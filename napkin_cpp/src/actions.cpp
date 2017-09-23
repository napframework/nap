#include "actions.h"

#include <QApplication>


OpenFileAction::OpenFileAction() : QAction("Open...") {
    setShortcut(QKeySequence::Open);
    connect(this, &QAction::triggered, this, &OpenFileAction::perform);
}

void OpenFileAction::perform() {
    auto lastFilename = AppContext::get().lastOpenedFilename();
    QString filename = QFileDialog::getOpenFileName(QApplication::topLevelWidgets()[0],
                                                    "Open NAP Data File",
                                                    lastFilename,
                                                    NAP_FILE_FILTER);
    if (filename.isNull())
        return;

    AppContext::get().loadFile(filename);
}

SaveFileAction::SaveFileAction() : QAction("Save") {
    setShortcut(QKeySequence::Save);
    connect(this, &QAction::triggered, this, &SaveFileAction::perform);
}

void SaveFileAction::perform() {
    if (AppContext::get().currentFilename().isNull()) {
        SaveFileAsAction().trigger();
        return;
    }
    AppContext::get().saveFile();
}

SaveFileAsAction::SaveFileAsAction() : QAction("Save as...") {
    setShortcut(QKeySequence::SaveAs);
    connect(this, &QAction::triggered, this, &SaveFileAsAction::perform);
}

void SaveFileAsAction::perform() {
    auto& ctx = AppContext::get();
    auto prevFilename = ctx.currentFilename();
    if (prevFilename.isNull())
        prevFilename = ctx.lastOpenedFilename();

    QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0],
                                                    "Save NAP Data File", prevFilename, NAP_FILE_FILTER);

    if (filename.isNull())
        return;

    ctx.saveFileAs(filename);
}

