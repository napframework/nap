/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <napqt/errordialog.h>

// Local includes
#include "actionmodel.h"
#include "appcontext.h"
#include "themeselectionmenu.h"
#include "stagewidget.h"
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
#include "panels/pathbrowserpanel.h"

namespace napkin
{
	/**
	 * The main application window. It will spawn and keep all the application's panels.
	 * Our application's data is managed by AppContext.
	 */
	class MainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		MainWindow();
		virtual ~MainWindow();

	protected:
		void showEvent(QShowEvent* event) override;
		void hideEvent(QHideEvent* event) override;
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
		 * Add the menu bar
		 */
		void addToolstrip();

		/**
		 * Called when a new dock widget is activated.
		 */
		void onDocked(QDockWidget *dockWidget);

		/**
		 * Configure the menu
		 */
		void configureMenu();

		/**
		 * Makes the window title up to date
		 */
		void updateWindowTitle();

		/**
		 * Called when a document is opened
		 * @param filename The filename of the newly opened document
		 */
		void onDocumentOpened(const QString& filename);

		/**
		 * Called when a file has been opened
		 * @param filename The file that has been opened
		 */
		void onDocumentChanged();

		/**
		 * Called when the selection changes
		 * @param paths The newly selected objects
		 */
		void onResourceSelectionChanged(const QList<PropertyPath>& paths);

		/**
		 * Occurs when a stage (load) request is issued
		 * @param path the path to load
		 * @param selection the selected widget
		 */
		void onStageRequested(const PropertyPath& path, const StageOption& selection);

		/**
		 * Handled when the scenepanel's selection changes
		 * @param paths The paths that have been selected
		 */
		void onSceneSelectionChanged(const QList<PropertyPath>& paths);

		/**
		 * Handled when the service config panel selection changes
		 */
		void onServiceConfigChanged(const QList<PropertyPath>& paths);

		/**
		 * Handled when a scene component selection was requested
		 * @param path
		 */
		void onSceneComponentSelectionRequested(nap::RootEntity* rootEntity, const QString& path);
		/**
		 * Receive messages from the logger
		 * @param msg The log message being emitted
		 */
		void onLog(const nap::LogMessage& msg);

		/**
		 * Called when an application-wide blocking operation started, progresses or finishes
		 * @param fraction progress fraction
		 * @param message message to display
		 */
		void onProgress(float fraction, const QString& message);

		/**
		 * Called when a project loaded
		 */
		void onProjectLoaded(const nap::ProjectInfo& projectInfo);

		/**
		 * Enabled / Disable project dependent actions
		 * @param enable / disable project dependent actions
		 */
		void enableProjectDependentActions(bool enable);

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
		 * Synchronize dock toggle actions
		 */
		void rebuildDockMenu();

		/**
		 * @return The application context, providing access to the application's content state
		 */
		AppContext& getContext() const;

		/**
		 * Add a QWidget to this window, display it as a dockwidget and add a toggle menuitem to the menubar
		 * @param name The name and title of the specified widget.
		 * @param widget The widget to be added as a dockwidget.
		 * @param area The initial area to stick the the dock into.
		 */
		 QDockWidget* addDock(const QString& name, QWidget* widget, Qt::DockWidgetArea area = Qt::TopDockWidgetArea);

	private:
		bool mShown = false;
		ActionModel mActionModel;
		ResourcePanel mResourcePanel;
		InspectorPanel mInspectorPanel;
		HistoryPanel mHistoryPanel;
		ModulePanel mModulePanel;
		InstancePropPanel mInstPropPanel;
		LogPanel mLogPanel;
		AppRunnerPanel mAppRunnerPanel;
		CurvePanel mCurvePanel;
		ScenePanel mScenePanel;
		std::vector<std::unique_ptr<StageWidget>> mApplets;
		ServiceConfigPanel mServiceConfigPanel;

		ThemeSelectionMenu mThemeMenu;
		QMenu mProjectMenu;
		QMenu mFileMenu;
		QMenu mConfigMenu;
		QMenu mCreateMenu;
		QMenu mHelpMenu;
		QMenu mPanelsMenu;
		QMenu mRecentProjectsMenu;
		nap::qt::ErrorDialog mErrorDialog;
		QStatusBar mStatusBar;
		QTimer mTimer;
		QToolBar* mToolbar = nullptr;
		std::unique_ptr<QProgressDialog> mProgressDialog = nullptr;
	};
};
