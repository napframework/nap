#pragma once

#include "appcontext.h"
#include <QHBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QThread>
#include <QWidget>
#include <generic/fileselector.h>
#include <generic/utility.h>


#define LAST_CORE_APP "lastCoreApp"

#define TXT_START "Start"
#define TXT_STOP "Stop"

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

	protected:
		/**
		 * Reimplemented from QWidget
		 */
		void showEvent(QShowEvent* event) override;

	private:
		/**
		 * QProcess handler
		 */
		void onAppChanged(const QString& filename);

		/**
		 * QProcess handler
		 */
		void onStartApp();

		/**
		 * QProcess handler
		 */
		void onStopApp();

		/**
		 * QProcess handler
		 */
		void onReadOut();

		/**
		 * QProcess handler
		 */
		void onReadErr();

		/**
		 * QProcess handler
		 */
		void onAppStarted();

		/**
		 * QProcess handler
		 */
		void onAppError(QProcess::ProcessError error);

		/**
		 * QProcess handler
		 */
		void onAppState(QProcess::ProcessState state);

		/**
		 * QProcess handler
		 */
		void onAppFinished(int exitCode);

		FileSelector mFileSelector; // Widget allowing you to select a file
		QHBoxLayout mLayout;		// Layout
		QPushButton mStartButton;   // Button with "Start" on it
		QPushButton mStopButton;	// Button with "Stop" on it
		QProcess mProcess;			// A handle to the nap application process
	};
};