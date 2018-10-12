#include "basewindow.h"
#include "autosettings.h"


napkin::BaseWindow::BaseWindow()
{
	setWindowTitle(QApplication::applicationName());
	setDockNestingEnabled(true);
	mWindowMenu = new QMenu("Window");
	menuBar()->addMenu(mWindowMenu);
}

QDockWidget* napkin::BaseWindow::addDock(const QString& name, QWidget* widget, Qt::DockWidgetArea area)
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

void napkin::BaseWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	AutoSettings::get().restore(*this);
}

void napkin::BaseWindow::closeEvent(QCloseEvent* event)
{
	AutoSettings::get().store(*this);
	QWidget::closeEvent(event);
}

