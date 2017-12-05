#pragma once

#include <QWidget>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <generic/filtertreeview.h>
#include <nap/logger.h>

namespace napkin
{
	/**
	 * Captures log messages and provides the LogPanel with the currently cached log messages.
	 */
	class LogModel : public QStandardItemModel
	{
		Q_OBJECT
	public:
		LogModel();

	Q_SIGNALS:

		/**
		 * Will be used to relay thread-unsafe nap::Logger calls onto the Qt UI thread
		 * @param msg The log message being handled
		 */
		void napLogged(nap::LogMessage msg);

	private:
		/**
		 * Signals from napLogged() will arrive on this handler.
		 * @param log The log mesage
		 */
		void onLog(nap::LogMessage log);

		int mMaxRows = 1000; // The maximum number of rows to show in the log
	};

	/**
	 * A panel showing all log messages in the system.
	 */
	class LogPanel : public QWidget
	{
	public:
		explicit LogPanel();

	private:
		FilterTreeView mTreeView; // Treeview with log entries
		LogModel mLogModel;		  // The model containing the log entries
	};
};