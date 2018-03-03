#pragma once

#include <QStatusBar>

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
#include "panels/timeline/timelinepanel.h"
#include "themeselectionmenu.h"

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
		void onResourceSelectionChanged(QList<nap::rtti::RTTIObject*> objects);

	private:
		ResourcePanel mResourcePanel;	// ResourcePanel
		HierarchyPanel mHierarchyPanel; // HierarchyPanel
		InspectorPanel mInspectorPanel; // InspectorPanel
		HistoryPanel mHistoryPanel;		// HistoryPanel
		LogPanel mLogPanel;				// LogPanel
		AppRunnerPanel mAppRunnerPanel; // AppRunnerPanel
		ThemeSelectionMenu mThemeMenu;  // ThemeSelectionMenu
		ScenePanel mScenePanel;			// ScenePanel
		TimelinePanel mTimelinePanel; 	// Timeline Panel

		QStatusBar mStatusBar;			// Status bar
	};
};