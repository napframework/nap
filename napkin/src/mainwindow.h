#pragma once

#include <generic/basewindow.h>
#include "outlinepanel.h"
#include "actions.h"
#include "hierarchypanel.h"
#include "appcontext.h"
#include "inspectorpanel.h"
#include "historypanel.h"
#include "logpanel.h"
#include "apprunnerpanel.h"
#include "themeselectionmenu.h"

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

private:
    void onNewFile();
    void onFileOpened(const QString& filename);
    void onFileSaved(const QString& filename);
    void updateWindowTitle();

private:
    OutlinePanel mOutlinePanel;
    HierarchyPanel mHierarchyPanel;
    InspectorPanel mInspectorPanel;
    HistoryPanel mHistoryPanel;
    LogPanel mLogPanel;
    AppRunnerPanel mAppRunnerPanel;

    ThemeSelectionMenu mThemeMenu;
};

