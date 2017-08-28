#include "mainwindow.h"

#include <QApplication>

void MainWindow::bindSignals() {
    connect(&AppContext::instance(), &AppContext::fileOpened, this, &MainWindow::onFileOpened);
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
    setWindowTitle(QString("%1 - %2").arg(QApplication::applicationName(), filename));
}
