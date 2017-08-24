#pragma once
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QDockWidget>
#include <QMenuBar>


#define WIN_GEO "WindowGeometry"
#define WIN_STATE "WindowState"

class BaseWindow : public QMainWindow {
public:
    BaseWindow() : QMainWindow() {
        setWindowTitle(QApplication::applicationName());
        setDockNestingEnabled(true);
        mWindowMenu = new QMenu("Window");
        menuBar()->addMenu(mWindowMenu);
    }

    /**
     * Add a QWidget to this window, display it as a dockwidget and add a toggle menuitem to the menubar
     * @param name The name and title of the specified widget.
     * @param widget The widget to be added as a dockwidget.
     * @param area The initial area to stick the the dock into.
     */
    QDockWidget* addDock(const QString &name, QWidget* widget, Qt::DockWidgetArea area = Qt::TopDockWidgetArea) {
        auto dock = new QDockWidget(this);
        dock->setObjectName(name);
        dock->setWidget(widget);
        dock->setWindowTitle(name);

        auto action = mWindowMenu->addAction(name);
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, &QAction::triggered, [dock, action]() {
            dock->setVisible(action->isChecked());
        });
        addDockWidget(area, dock);
        return dock;
    }

protected:
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        QSettings s;
        restoreGeometry(s.value(WIN_GEO).toByteArray());
        restoreState(s.value(WIN_STATE).toByteArray());
    }

    void closeEvent(QCloseEvent* event) override {
        QSettings s;
        s.setValue(WIN_STATE, saveState());
        s.setValue(WIN_GEO, saveGeometry());
        QWidget::closeEvent(event);
    }


private:
    QMenu* mWindowMenu;
};