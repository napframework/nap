#include "basewindow.h"

#include <QApplication>

#include "autosettings.h"

napqt::BaseWindow::BaseWindow()
{
	setWindowTitle(QApplication::applicationName());
	setDockNestingEnabled(true);
	mWindowMenu = new QMenu("Window");
	menuBar()->addMenu(mWindowMenu);
}

QDockWidget* napqt::BaseWindow::addDock(const QString& name, QWidget* widget, Qt::DockWidgetArea area)
{
	QDockWidget* dock = new QDockWidget(this);
	dock->setObjectName(name);
	dock->setWidget(widget);
	dock->setWindowTitle(name);

	if (widget->objectName().isEmpty())
		widget->setObjectName(QString("%1_Widget").arg(name));

	QAction* action = mWindowMenu->addAction(name);
	action->setCheckable(true);
	action->setChecked(true);
	connect(action, &QAction::triggered, [dock, action]() { dock->setVisible(action->isChecked()); });
	addDockWidget(area, dock);
	return dock;
}

void napqt::BaseWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	AutoSettings::get().restore(*this);
}

void napqt::BaseWindow::closeEvent(QCloseEvent* event)
{
	AutoSettings::get().store(*this);
	QWidget::closeEvent(event);
}

