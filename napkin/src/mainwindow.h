#pragma once

#include <QStatusBar>
#include <QTimer>

#include <napqt/basewindow.h>
#include <napqt/timeline/timelinepanel.h>
#include <napqt/errordialog.h>

#include "actions.h"
#include "appcontext.h"
#include "panels/apprunnerpanel.h"
#include "panels/hierarchypanel.h"
#include "panels/historypanel.h"
#include "panels/inspectorpanel.h"
#include "panels/logpanel.h"
#include "panels/resourcepanel.h"
#include "panels/scenepanel.h"
#include "panels/curvepanel.h"
#include "themeselectionmenu.h"

namespace napkin
{
	/**
	 * The main application window. It will spawn and keep all the application's panels.
	 * Our application's data is managed by AppContext.
	 */
	class MainWindow : public napqt::BaseWindow
	{
		Q_OBJECT
	public:
		MainWindow();
		virtual ~MainWindow();

	protected:
		/**
		 * Override...
		 */
		void showEvent(QShowEvent* event) override;

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
		 * @param objects The newly selected objects
		 */
		void onResourceSelectionChanged(QList<nap::rtti::Object*> objects);

		/**
		 * Receive messages from the logger
		 * @param msg The log message being emitted
		 */
		void onLog(nap::LogMessage msg);

		/**
		 * Show a logmessage in the error dialog
		 * @param msg The message to be displayed
		 */
		void showError(nap::LogMessage msg);

	private:
		ResourcePanel mResourcePanel;
		HierarchyPanel mHierarchyPanel;
		InspectorPanel mInspectorPanel;
		HistoryPanel mHistoryPanel;
		LogPanel mLogPanel;
		AppRunnerPanel mAppRunnerPanel;
		CurvePanel mCurvePanel;
		ThemeSelectionMenu mThemeMenu;
		ScenePanel mScenePanel;
		napqt::ErrorDialog mErrorDialog;
		QStatusBar mStatusBar;
		QTimer mTimer;
	};
};
