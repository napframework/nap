#pragma once

#include <generic/basewindow.h>
#include "outlinepanel.h"
#include "actions.h"
#include "hierarchypanel.h"
#include "appcontext.h"

class MainWindow : public BaseWindow {
    Q_OBJECT
public:
    MainWindow() {
        addDocks();
        addMenu();
        bindSignals();
    }
    virtual ~MainWindow() {}

private:
    void bindSignals();

    void addDocks();

    void addMenu();

private:
    void onFileOpened(const QString& filename);

private:
    OutlinePanel outlinePanel;
    HierarchyPanel hierarchyPanel;
};

