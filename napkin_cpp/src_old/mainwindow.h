#pragma once

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QtCore/QFileSystemWatcher>
#include <QtWidgets/QDockWidget>

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = 0);
	~MainWindow();
	void restoreGUI();
	void closeEvent(QCloseEvent* event) override;

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onSceneChanged();

private:
    void initPanels();
    void initMenu();
    void loadStyleSheet();
    void updateWindowTitle();
    QMenu* getOrCreateMenu(QString name);
	void initPanel(QDockWidget* panel, Qt::DockWidgetArea area, const QIcon* icon = nullptr);

	Ui::MainWindowClass ui;
	QFileSystemWatcher mResourceWatcher;
};
