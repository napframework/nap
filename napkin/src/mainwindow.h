#pragma once

#include "actions.h"
#include "appcontext.h"
#include "generic/basewindow.h"
#include "panels/apprunnerpanel.h"
#include "panels/hierarchypanel.h"
#include "panels/historypanel.h"
#include "panels/inspectorpanel.h"
#include "panels/logpanel.h"
#include "panels/resourcepanel.h"
#include "themeselectionmenu.h"
#include <panels/scenepanel.h>

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
		 * BInd signals
		 */
		void bindSignals();

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
		 * Called when a file has been opened
		 * @param filename The file that has been opened
		 */
		void onDocumentChanged();

	private:
		ResourcePanel mResourcePanel;	// ResourcePanel
		HierarchyPanel mHierarchyPanel; // HierarchyPanel
		InspectorPanel mInspectorPanel; // InspectorPanel
		HistoryPanel mHistoryPanel;		// HistoryPanel
		LogPanel mLogPanel;				// LogPanel
		AppRunnerPanel mAppRunnerPanel; // AppRunnerPanel
		ThemeSelectionMenu mThemeMenu;  // ThemeSelectionMenu
		ScenePanel mScenePanel;			// ScenePanel
	};
};