/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apprunnerpanel.h"
#include "napkin-resources.h"
#include "napkinutils.h"
#include "napkinglobals.h"
#include "naputils.h"

#include <nap/logger.h>
#include <napqt/qtutils.h>
#include <QDirIterator>

napkin::AppRunnerPanel::AppRunnerPanel() : QWidget()
{
	setLayout(&mLayout);
	themeChanged(nullptr);
	mStopButton.setEnabled(false);

	mLayout.addWidget(&mFileSelector);
	mLayout.addWidget(&mStartButton);
	mLayout.addWidget(&mStopButton);

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

	// When project loads find executable and set it
	connect(&AppContext::get(), &AppContext::projectLoaded, this, &AppRunnerPanel::onProjectLoaded);
}

napkin::AppRunnerPanel::~AppRunnerPanel()
{
	onStopApp();
}


static QString sGetBuildDir(const nap::ProjectInfo& projectInfo)
{
	// Find root based on context
	const auto& napkin_ctx = napkin::utility::Context::get();
	QString root = napkin_ctx.getType() == napkin::utility::Context::EType::Source ?
		QString::fromStdString(projectInfo.getNAPRootDir()) :
		QString::fromStdString(projectInfo.getProjectDir());
    assert(!root.isEmpty());

	// Build dir is the installation directory when dealing with packaged app
	if (napkin_ctx.getType() == napkin::utility::Context::EType::Application)
		return root;

	// Check if the bin dir exists
	QFileInfo bin_dir(root, "bin");
	if (!bin_dir.exists() || !bin_dir.isDir())
		return "";

	// Locate final build directory
    QString build_output = "";
	QDirIterator it(bin_dir.filePath(), QDir::Dirs | QDir::NoDotAndDotDot);
	QString napkin_build_type = QString(napkin::BUILD_TYPE).toLower();
	while (it.hasNext())
	{
		QString child_dir = it.next();
		if (child_dir.toLower().contains(napkin_build_type))
		{
			build_output = child_dir;
			break;
		}
	}
	return build_output;
}


void napkin::AppRunnerPanel::onProjectLoaded(const nap::ProjectInfo& projectInfo)
{
	// Find build output directory
	auto build_dir = sGetBuildDir(projectInfo);
	if (build_dir.isEmpty())
	{
		nap::Logger::warn("Unable to locate '%s' build directory", projectInfo.mTitle.c_str());
		mFileSelector.setFilename("");
		return;
	}

	// Find executable
	QString exe_file = QString::fromStdString(projectInfo.mTitle).toLower();
	QDirIterator it(build_dir, QDir::Files | QDir::NoDotAndDotDot);
	while (it.hasNext())
	{
		// Peek and use
        it.next();
        if (it.fileInfo().isExecutable() && it.fileName().toLower().startsWith(exe_file))
		{
			nap::Logger::info("Setting executable '%s'",
				napkin::toLocalURI(it.filePath().toStdString()).c_str());
			mFileSelector.setFilename(it.filePath());
			return;
		}
	}
	nap::Logger::warn("Unable to find '%s' executable", projectInfo.mTitle.c_str());
	mFileSelector.setFilename("");
}


void napkin::AppRunnerPanel::onStartApp()
{
	QString executable = mFileSelector.getFilename();

	if (!QFileInfo::exists(executable))
	{
		nap::Logger::fatal(executable.isEmpty() ? "No executable specified" :
			"Executable not found: '%s'", executable.toStdString().c_str());
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
