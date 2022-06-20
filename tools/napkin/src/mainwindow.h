/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QStatusBar>
#include <QTimer>

#include <napqt/basewindow.h>
#include <napqt/errordialog.h>
#include <panels/pathbrowserpanel.h>

#include "actions.h"
#include "appcontext.h"
#include "themeselectionmenu.h"

#include "panels/apprunnerpanel.h"
#include "panels/historypanel.h"
#include "panels/inspectorpanel.h"
#include "panels/logpanel.h"
#include "panels/resourcepanel.h"
#include "panels/scenepanel.h"
#include "panels/curvepanel.h"
#include "panels/modulepanel.h"
#include "panels/instanceproppanel.h"
#include "panels/serviceconfigpanel.h"

namespace napkin
{
	/**
	 * The main application window. It will spawn and keep all the application's panels.
	 * Our application's data is managed by AppContext.
	 */
	class MainWindow : public nap::qt::BaseWindow
	{
		Q_OBJECT
	public:
		MainWindow();
		virtual ~MainWindow();
	protected:
		/**
		 * Override
		 */
		void showEvent(QShowEvent* event) override;

		/**
		 * Hide event
		 */
		void hideEvent(QHideEvent* event) override;

		/**
		 * Override
		 */
		void closeEvent(QCloseEvent* event) override;

	private:
		/**
		 * Bind signals
		 */
		void bindSignals();

		/**
		 * Disconnect signals
		 */
		void unbindSignals();

		/**
		 * Add all the docks/panels
		 */
		void addDocks();

		/**
		 * Called when a new dock widget is activated.
		 */
		void onDocked(QDockWidget *dockWidget);

		/**
		 * Add the menu
		 */
		void addMenu();

		/**
		 * Makes the window title up to date
		 */
		void updateWindowTitle();

		/**
		 * Called when a document is opened
		 * @param filename The filename of the newly opened document
		 */
		void onDocumentOpened(const QString filename);

		/**
		 * Called when a file has been opened
		 * @param filename The file that has been opened
		 */
		void onDocumentChanged();

		/**
		 * Called when the selection changes
		 * @param paths The newly selected objects
		 */
		void onResourceSelectionChanged(QList<PropertyPath> paths);

		/**
		 * Handled when the scenepanel's selection changes
		 * @param paths The paths that have been selected
		 */
		void onSceneSelectionChanged(QList<PropertyPath> paths);

		/**
		 * Handled when the service config panel selection changes
		 */
		void onServiceConfigChanged(QList<PropertyPath> paths);

		/**
		 * Handled when a scene component selection was requested
		 * @param path
		 */
		void onSceneComponentSelectionRequested(nap::RootEntity* rootEntity, const QString& path);
		/**
		 * Receive messages from the logger
		 * @param msg The log message being emitted
		 */
		void onLog(nap::LogMessage msg);

		/**
		 * Called when an application-wide blocking operation started, progresses or finishes
		 * @param fraction progress fraction
		 * @param message message to display
		 */
		void onProgress(float fraction, const QString& message);

		/**
		 * Show a logmessage in the error dialog
		 * @param msg The message to be displayed
		 */
		void showError(nap::LogMessage msg);

		/**
		 * If the current file is dirty, offer to save the file.
		 * @return False if the operation was cancelled,
		 * 		   true if the file was saved or if the user declined saving
		 */
		bool confirmSaveCurrentFile();

		/**
		 * Reconstruct the "recent files" menu
		 */
		void rebuildRecentMenu();

		/**
		 * @return The application context, providing access to the application's content state
		 */
		AppContext& getContext() const;

		/**
		 * Occurs when theme changes
		 */
		void onThemeChanged(const Theme* theme);

	private:
		bool mShown = false;
		ResourcePanel mResourcePanel;
//		PathBrowserPanel mPathBrowser;
		InspectorPanel mInspectorPanel;
		HistoryPanel mHistoryPanel;
		ModulePanel mModulePanel;
		InstancePropPanel mInstPropPanel;
		LogPanel mLogPanel;
		AppRunnerPanel mAppRunnerPanel;
		CurvePanel mCurvePanel;
		ServiceConfigPanel mServiceConfigPanel;
		ThemeSelectionMenu mThemeMenu;
		ScenePanel mScenePanel;
		QMenu* mRecentProjectsMenu = nullptr;
		nap::qt::ErrorDialog mErrorDialog;
		QStatusBar mStatusBar;
		QTimer mTimer;
		std::unique_ptr<QProgressDialog> mProgressDialog = nullptr;
	};
};
