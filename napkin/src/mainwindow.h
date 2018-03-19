#pragma once

#include <QStatusBar>
#include <QTimer>

#include "actions.h"

#include "appcontext.h"
#include "generic/basewindow.h"
#include "panels/apprunnerpanel.h"
#include "panels/hierarchypanel.h"
#include "panels/historypanel.h"
#include "panels/inspectorpanel.h"
#include "panels/logpanel.h"
#include "panels/resourcepanel.h"
#include "panels/scenepanel.h"
#include "themeselectionmenu.h"
#include "generic/errordialog.h"

namespace napkin
{
	/**
	 * The main application window. It will spawn and keep all the application's panels.
	 * Our application's data is managed by AppContext.
	 */
	class MainWindow : public BaseWindow
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
		ResourcePanel mResourcePanel;	// ResourcePanel
		HierarchyPanel mHierarchyPanel; // HierarchyPanel
		InspectorPanel mInspectorPanel; // InspectorPanel
		HistoryPanel mHistoryPanel;		// HistoryPanel
		LogPanel mLogPanel;				// LogPanel
		AppRunnerPanel mAppRunnerPanel; // AppRunnerPanel
		ThemeSelectionMenu mThemeMenu;  // ThemeSelectionMenu
		ScenePanel mScenePanel;			// ScenePanel
		ErrorDialog mErrorDialog;       // The error dialog, if there was an error
		QStatusBar mStatusBar;			// Status bar
		QTimer mTimer;
	};
};
