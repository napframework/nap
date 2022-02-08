/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apprunnerpanel.h"
#include "napkin-resources.h"

#include <nap/logger.h>
#include <napqt/qtutils.h>

napkin::AppRunnerPanel::AppRunnerPanel() : QWidget()
{
	setLayout(&mLayout);
	themeChanged(nullptr);
	mStopButton.setEnabled(false);

	mLayout.addWidget(&mFileSelector);
	mLayout.addWidget(&mStartButton);
	mLayout.addWidget(&mStopButton);

	connect(&mFileSelector, &nap::qt::FileSelector::filenameChanged, this, &AppRunnerPanel::onAppChanged);
	connect(&mStartButton, &QPushButton::clicked, this, &AppRunnerPanel::onStartApp);
	connect(&mStopButton, &QPushButton::clicked, this, &AppRunnerPanel::onStopApp);
	connect(&mProcess, &QProcess::started,	this, &AppRunnerPanel::onAppStarted);
	connect(&mProcess, &QProcess::errorOccurred, this, &AppRunnerPanel::onAppError);
	connect(&mProcess, &QProcess::stateChanged, this, &AppRunnerPanel::onAppState);
	connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AppRunnerPanel::onReadOut);
	connect(&mProcess, &QProcess::readyReadStandardError, this, &AppRunnerPanel::onReadErr);

	// Cast to tell the compiler which override to use
	connect(&mProcess, (void (QProcess::*)(int, QProcess::ExitStatus)) & QProcess::finished, this, &AppRunnerPanel::onAppFinished);

	// When theme changes, update icons
	connect(&AppContext::get().getThemeManager(), &ThemeManager::themeChanged, this, &AppRunnerPanel::themeChanged);
}

napkin::AppRunnerPanel::~AppRunnerPanel()
{
	onStopApp();
}


void napkin::AppRunnerPanel::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	mFileSelector.setFilename(QSettings().value(LAST_CORE_APP).toString());
}

void napkin::AppRunnerPanel::onAppChanged(const QString& filename)
{
	QSettings().setValue(LAST_CORE_APP, filename);
}

void napkin::AppRunnerPanel::onStartApp()
{
	QString executable = mFileSelector.getFilename();

	if (!QFileInfo::exists(executable))
	{
		nap::Logger::fatal("File not found: '%s'", executable.toStdString().c_str());
		return;
	}

	QStringList args;
	if (AppContext::get().getDocument() != nullptr)
		args << AppContext::get().getDocument()->getFilename();

	nap::Logger::info("Running: '%s %s'", executable.toStdString().c_str(), args.join(" ").toStdString().c_str());
	mStartButton.setEnabled(false);
	mProcess.start(executable, args);
}


void napkin::AppRunnerPanel::onStopApp()
{
	mProcess.terminate();
}


void napkin::AppRunnerPanel::onReadOut()
{
	QString out = mProcess.readAllStandardOutput().trimmed();
	for (auto line : out.split("\n"))
		nap::Logger::info(QString("[NAP] %1").arg(line).toStdString());
}


void napkin::AppRunnerPanel::onReadErr()
{
	QString err = mProcess.readAllStandardError().trimmed();
	if (err.contains(QString::fromStdString(nap::Logger::fatalLevel().name())))
	{
		for (auto line : err.split("\n"))
			nap::Logger::fatal(QString("[NAP] %1").arg(line).toStdString());
	}
	else
	{
		for (auto line : err.split("\n"))
			nap::Logger::error(QString("[NAP] %1").arg(line).toStdString());
	}
}


void napkin::AppRunnerPanel::onAppStarted()
{
	mStopButton.setEnabled(true);
}


void napkin::AppRunnerPanel::onAppError(QProcess::ProcessError error)
{
	nap::Logger::fatal(nap::qt::QEnumToString(error).toStdString());
	mStartButton.setEnabled(true);
	mStopButton.setEnabled(false);
}


void napkin::AppRunnerPanel::onAppState(QProcess::ProcessState state)
{
	nap::Logger::info("App State Changed: %s", nap::qt::QEnumToString(state).toStdString().c_str());
}

void napkin::AppRunnerPanel::onAppFinished(int exitCode, QProcess::ExitStatus)
{
	nap::Logger::info(QString("App Exit with code %1").arg(exitCode).toStdString());
	mStartButton.setEnabled(true);
	mStopButton.setEnabled(false);
}


void napkin::AppRunnerPanel::themeChanged(const Theme* theme)
{
	mStartButton.setIcon(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_PLAY_APP));
	mStopButton.setIcon(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_STOP_APP));
}
