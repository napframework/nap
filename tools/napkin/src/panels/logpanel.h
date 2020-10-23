/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSettings>

#include <nap/logger.h>

#include <napqt/filtertreeview.h>
#include <napqt/autosettings.h>

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

		/**
		 * Overridden to set log colors
		 */
		QVariant data(const QModelIndex& index, int role) const override;

	private:
		/**
		 * Signals from napLogged() will arrive on this handler.
		 * @param msg The log mesage
		 */
		void onLog(nap::LogMessage msg);

	private:
		int mMaxRows = 1000; // The maximum number of rows to show in the log
	};

	/**
	 * A panel showing all log messages in the system.
	 */
	class LogPanel : public QWidget
	{
		Q_OBJECT
	public:
		explicit LogPanel();
		const nap::LogLevel& getCurrentLevel() const;
		void setCurrentLevel(const nap::LogLevel& level);
		int getLevelIndex(const nap::LogLevel& level) const;

	protected:
		void closeEvent(QCloseEvent* event) override;
		void showEvent(QShowEvent* event) override;

	private:
		void populateFilterCombo();
		/**
		 * Provide FilterTreeView with a way of filtering log messages based on our level
		 */
		bool levelFilter(const nap::qt::LeafFilterProxyModel& model, int sourceRow, const QModelIndex& sourceParent);

		void onLevelChanged(int index);
		void onDoubleClicked(const QModelIndex& index);
		void onRowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
		void onRowInserted(const QModelIndex &parent, int first, int last);

		nap::qt::FilterTreeView mTreeView; 	// Treeview with log entries
		QVBoxLayout mLayout;		// The main layout
		QHBoxLayout mCornerLayout; 	// Layout at the top-right corner
		QComboBox mFilterCombo;		// Combo containing the log levels to filter on
		LogModel mLogModel;		  	// The model containing the log entries
		bool wasMaxScroll = true; 	// Whether the scroll view was at max
	};

	class LogPanelWidgetStorer : public nap::qt::WidgetStorer<LogPanel>
	{
	public:
		void store(const LogPanel& widget, const QString& key, QSettings& s) const override;
		void restore(LogPanel& widget, const QString& key, const QSettings& s) const override;
	};

};