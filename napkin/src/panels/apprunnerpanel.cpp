#include "apprunnerpanel.h"


napkin::AppRunnerPanel::AppRunnerPanel() : QWidget()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);

	mStartButton.setText(TXT_START);
	mStopButton.setText(TXT_STOP);
	mStopButton.setEnabled(false);

	mLayout.addWidget(&mFileSelector);
	mLayout.addWidget(&mStartButton);
	mLayout.addWidget(&mStopButton);

	//    mLayout.addStretch(1);

	connect(&mFileSelector, &FileSelector::filenameChanged, this, &AppRunnerPanel::onAppChanged);

	connect(&mStartButton, &QPushButton::clicked, this, &AppRunnerPanel::onStartApp);
	connect(&mStopButton, &QPushButton::clicked, this, &AppRunnerPanel::onStopApp);

	connect(&mProcess, &QProcess::started, this, &AppRunnerPanel::onAppStarted);
	connect(&mProcess, &QProcess::errorOccurred, this, &AppRunnerPanel::onAppError);
	connect(&mProcess, &QProcess::stateChanged, this, &AppRunnerPanel::onAppState);
	connect(&mProcess, &QProcess::readyReadStandardOutput, this, &AppRunnerPanel::onReadOut);
	connect(&mProcess, &QProcess::readyReadStandardError, this, &AppRunnerPanel::onReadErr);
	// Cast to tell the compiler which override to use
	connect(&mProcess, (void (QProcess::*)(int)) & QProcess::finished, this, &AppRunnerPanel::onAppFinished);
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
	QString executable = mFileSelector.filename();

	if (!QFileInfo::exists(executable))
	{
		nap::Logger::fatal("File not found: '%s'", executable.toStdString().c_str());
		return;
	}

	QStringList args;
	args << AppContext::get().currentFilename();

	nap::Logger::info("Running: \"%s %s\"", executable.toStdString().c_str(), args.join(" ").toStdString().c_str());

	mStartButton.setEnabled(false);

	mProcess.start(executable, args);
}

void napkin::AppRunnerPanel::onStopApp()
{
	mProcess.close();
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
	for (auto line : err.split("\n"))
		nap::Logger::fatal(QString("[NAP] %1").arg(line).toStdString());
}

void napkin::AppRunnerPanel::onAppStarted()
{
	mStopButton.setEnabled(true);
}

void napkin::AppRunnerPanel::onAppError(QProcess::ProcessError error)
{
	nap::Logger::fatal(QEnumToString(error).toStdString());
	mStartButton.setEnabled(true);
	mStopButton.setEnabled(false);
}

void napkin::AppRunnerPanel::onAppState(QProcess::ProcessState state)
{
	nap::Logger::info("App State Changed: %s", QEnumToString(state).toStdString().c_str());
}

void napkin::AppRunnerPanel::onAppFinished(int exitCode)
{
	nap::Logger::info(QString("App Exit with code %1").arg(exitCode).toStdString());
	mStartButton.setEnabled(true);
	mStopButton.setEnabled(false);
}
