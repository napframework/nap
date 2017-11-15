#pragma once

#include <QMainWindow>
#include <QMenu>
#include <QDockWidget>
#include <QMenuBar>
#include "appcontext.h"


class BaseWindow : public QMainWindow {
//    Q_OBJECT
public:
    BaseWindow();
    virtual ~BaseWindow() {}

    /**
     * @return The QMenu that contains the list of available windows
     */
    QMenu* windowMenu() { return mWindowMenu; }

    /**
     * Add a QWidget to this window, display it as a dockwidget and add a toggle menuitem to the menubar
     * @param name The name and title of the specified widget.
     * @param widget The widget to be added as a dockwidget.
     * @param area The initial area to stick the the dock into.
     */
    QDockWidget* addDock(const QString& name, QWidget* widget, Qt::DockWidgetArea area = Qt::TopDockWidgetArea);

protected:
    void showEvent(QShowEvent* event) override;

    void closeEvent(QCloseEvent* event) override;


private:
    QMenu* mWindowMenu;
};