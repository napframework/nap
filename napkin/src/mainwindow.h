#pragma once

#include "actions.h"
#include "appcontext.h"
#include "panels/apprunnerpanel.h"
#include "panels/hierarchypanel.h"
#include "panels/historypanel.h"
#include "panels/inspectorpanel.h"
#include "panels/logpanel.h"
#include "panels/resourcepanel.h"
#include "themeselectionmenu.h"
#include "generic/basewindow.h"

/**
 * The main application window. It will spawn and keep all the application's panels.
 * Our application's data is managed by AppContext.
 */
class MainWindow : public BaseWindow {
    Q_OBJECT
public:
    MainWindow() {
        addDocks();
        addMenu();
        bindSignals();
    }

    virtual ~MainWindow() {}

protected:
    void showEvent(QShowEvent* event) override;

private:
    void bindSignals();
    void addDocks();
    void addMenu();
    void updateWindowTitle();

    void onNewFile();
    void onFileOpened(const QString& filename);
    void onFileSaved(const QString& filename);

private:
    ResourcePanel mOutlinePanel;
    HierarchyPanel mHierarchyPanel;
    InspectorPanel mInspectorPanel;
    HistoryPanel mHistoryPanel;
    LogPanel mLogPanel;
    AppRunnerPanel mAppRunnerPanel;
    ThemeSelectionMenu mThemeMenu;
};

