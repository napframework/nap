#include "mainwindow.h"

#include <QApplication>

void MainWindow::bindSignals() {
    connect(&AppContext::get(), &AppContext::fileOpened, this, &MainWindow::onFileOpened);
    connect(&AppContext::get(), &AppContext::fileSaved, this, &MainWindow::onFileSaved);
}

void MainWindow::addDocks() {
    addDock("Outline", &outlinePanel);
    addDock("Types", &hierarchyPanel);
}

void MainWindow::addMenu() {
    QMenu* filemenu = new QMenu("File", menuBar());

    auto openFileAction = new OpenFileAction(filemenu);
    addAction(openFileAction);
    filemenu->addAction(openFileAction);

    auto saveFileAction = new SaveFileAction(filemenu);
    addAction(saveFileAction);
    filemenu->addAction(saveFileAction);

    auto saveFileAsAction = new SaveFileAsAction(filemenu);
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

void MainWindow::updateWindowTitle() {
    setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName(), AppContext::get().currentFilename()));
}
