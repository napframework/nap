#include "mainwindow.h"

#include <QApplication>

void MainWindow::bindSignals() {
    connect(&AppContext::get(), &AppContext::fileOpened, this, &MainWindow::onFileOpened);
    connect(&AppContext::get(), &AppContext::fileSaved, this, &MainWindow::onFileSaved);

    connect(&mOutlinePanel, &OutlinePanel::selectionChanged, [&](QList<nap::rtti::RTTIObject*>& objects) {
        mInspectorPanel.setObject(objects.isEmpty() ? nullptr : objects.first());
    });
}

void MainWindow::showEvent(QShowEvent* event) {
    BaseWindow::showEvent(event);
    openRecentFile();
}


void MainWindow::addDocks() {
    addDock("Outline", &mOutlinePanel);
    addDock("Types", &mHierarchyPanel);
    addDock("Inspector", &mInspectorPanel);
}

void MainWindow::addMenu() {
    QMenu* filemenu = new QMenu("File", menuBar());

    auto openFileAction = new OpenFileAction();
    addAction(openFileAction);
    filemenu->addAction(openFileAction);

    auto saveFileAction = new SaveFileAction();
    addAction(saveFileAction);
    filemenu->addAction(saveFileAction);

    auto saveFileAsAction = new SaveFileAsAction();
    addAction(saveFileAction);
    filemenu->addAction(saveFileAsAction);

    menuBar()->insertMenu(windowMenu()->menuAction(), filemenu);
}

void MainWindow::onFileOpened(const QString& filename) {
    updateWindowTitle();
}

void MainWindow::onFileSaved(const QString& filename) {
    updateWindowTitle();
}

void MainWindow::openRecentFile() {
    auto lastFilename = AppContext::get().lastOpenedFilename();
    if (lastFilename.isNull())
        return;
    AppContext::get().loadFile(lastFilename);

}


void MainWindow::updateWindowTitle() {
    setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName(), AppContext::get().currentFilename()));
}


