#pragma once

#include <QWidget>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <generic/filtertreeview.h>
#include <nap/logger.h>


/**
 * Captures log messages and provides the LogPanel with the currently cached log messages.
 */
class LogModel : public QStandardItemModel
{
	Q_OBJECT
public:
	LogModel();

Q_SIGNALS:
	void napLogged(nap::LogMessage msg);

private:
	void onLog(nap::LogMessage log);

	int maxRows = 1000;
};


class LogPanel : public QWidget
{
public:
	explicit LogPanel();

private:
	FilterTreeView mTreeView;
	LogModel mLogModel;
};