/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "appcontext.h"

#include <QHBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QThread>
#include <QWidget>

#include <napqt/fileselector.h>

namespace napkin
{

	/**
	 * Through this panel, the user can select and start/stop a nap application.
	 * The standard output of the application will be redirected into the logger.
	 */
	class AppRunnerPanel : public QWidget
	{
		Q_OBJECT
	public:
		AppRunnerPanel();

		~AppRunnerPanel();

	private:
		/**
		 * Called when a project loaded successfully
		 */
		void onProjectLoaded(const nap::ProjectInfo& projectInfo);

		/**
		 * Called when the app is launched
		 */
		void onStartApp();

		/**
		 * Called when the app should be stopped
		 */
		void onStopApp();

		/**
		 * Read line output
		 */
		void onReadOut();

		/**
		 * Read error output
		 */
		void onReadErr();

		/**
		 * Called when the app launched successfully
		 */
		void onAppStarted();

		/**
		 * Called when the app produces an error through the cmdline
		 */
		void onAppError(QProcess::ProcessError error);

		/**
		 * Called when the state of the app changes
		 */
		void onAppState(QProcess::ProcessState state);

		/**
		 * Called when the app finishes execution
		 */
		void onAppFinished(int exitCode, QProcess::ExitStatus);

		/**
		 * Occurs when theme changes
		 */
		void themeChanged(const Theme* theme);

		nap::qt::FileSelector mFileSelector;	// Widget allowing you to select a file
		QHBoxLayout mLayout;					// Layout
		QPushButton mStartButton;				// Button with "Start" on it
		QPushButton mStopButton;				// Button with "Stop" on it
		QProcess mProcess;						// A handle to the nap application process
	};
};
