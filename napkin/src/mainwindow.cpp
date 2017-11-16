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
    addDock("Available Types", &mHierarchyPanel);
    addDock("Inspector", &mInspectorPanel);
    addDock("History", &mHistoryPanel);
    addDock("Log", &mLogPanel);
}

void MainWindow::addMenu() {
    auto filemenu = new QMenu("File", menuBar());
    {
        auto newFileAction = new NewFileAction();
        addAction(newFileAction);
        filemenu->addAction(newFileAction);

        auto openFileAction = new OpenFileAction();
        addAction(openFileAction);
        filemenu->addAction(openFileAction);

        auto saveFileAction = new SaveFileAction();
        addAction(saveFileAction);
        filemenu->addAction(saveFileAction);

        auto saveFileAsAction = new SaveFileAsAction();
        addAction(saveFileAction);
        filemenu->addAction(saveFileAsAction);
    }
    menuBar()->insertMenu(windowMenu()->menuAction(), filemenu);

    auto optionsMenu = new QMenu("Options", menuBar());
    {
        auto themeMenu = new QMenu("Theme", optionsMenu);
        {
            auto defaultThemeAction = new SetThemeAction(nullptr);
            themeMenu->addAction(defaultThemeAction);

            for (auto theme : AppContext::get().availableThemes())
                themeMenu->addAction(new SetThemeAction(theme));
        }
        optionsMenu->addMenu(themeMenu);

    }
    menuBar()->insertMenu(windowMenu()->menuAction(), optionsMenu);
}

void MainWindow::onNewFile()
{
    updateWindowTitle();
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



