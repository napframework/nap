#include <QFontDatabase>
#include <QCommandLineParser>

#include <utility/fileutils.h>

#include "mainwindow.h"
#include "napkin-resources.h"
#include "appcontext.h"

using namespace napkin;

/**
 * Configure in-between session settings.
 * If the application is started for the first time (ie. no user settings are found),
 * use the application's default settings as a starting point.
 */
void initializeSettings()
{
	auto exeDir = QString::fromStdString(nap::utility::getExecutableDir());
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, exeDir);
	QSettings::setDefaultFormat(QSettings::IniFormat);

	auto userSettingsFilename = QSettings().fileName();
	if (!QFileInfo::exists(userSettingsFilename))
	{
		auto settingsDir = QFileInfo(userSettingsFilename).dir();
		if (!settingsDir.exists())
			settingsDir.mkpath(".");

		auto defaultSettingsFilename = QString("%1/%2").arg(exeDir, DEFAULT_SETTINGS_FILE);
		if (!QFileInfo::exists(defaultSettingsFilename))
			nap::Logger::error("File not found: %s", defaultSettingsFilename.toStdString().c_str());
		if (!QFile::copy(defaultSettingsFilename, userSettingsFilename))
			nap::Logger::error("Failed to copy %s to %s",
							   defaultSettingsFilename.toStdString().c_str(),
							   userSettingsFilename.toStdString().c_str());
	}
}

/**
 * Initialize the application and spawn its window
 */
int main(int argc, char* argv[])
{
	// Start logging to file
	nap::Logger::logToDirectory(nap::utility::getExecutableDir() + "/log", "napkin");

    // Construct the app context singleton
    AppContext::create();

	// nap::Core is declared in AppContext
	QApplication::setOrganizationName("napframework");
	QApplication::setApplicationName("Napkin");
	QApplication::setApplicationVersion("0.1");

	initializeSettings();

	QApplication app(argc, argv);

	// handle commandline
	QCommandLineParser parser;
	auto opHelp = parser.addHelpOption();
	auto opVer = parser.addVersionOption();

	QCommandLineOption opProject({"p", "project"}, "Load specified project directory upon startup", "project", "");
	parser.addOption(opProject);
    // Options to assist with automated testing
	QCommandLineOption opExitFailure("exit-on-failure", "Exit on on failure loading project", "", "");
	parser.addOption(opExitFailure);
	QCommandLineOption opNoOpenRecent("no-project-reopen", "Don't attempt to re-open last project", "", "");
	parser.addOption(opNoOpenRecent);

	parser.process(app);

	if (parser.isSet(opHelp))
		return 0;

	if (parser.isSet(opVer))
		return 0;

	if (parser.isSet(opProject))
	{
		AppContext::get().addRecentlyOpenedProject(parser.value(opProject));
	}
	else if (parser.isSet(opNoOpenRecent))
	{
		AppContext::get().disableRecentProjectOpening();
	}

	if (parser.isSet(opExitFailure))
		AppContext::get().enableExitOnProjectLoadFailure();

	// Create main window and run
	app.setWindowIcon(QIcon(QRC_ICONS_NAP_LOGO));
	std::unique_ptr<MainWindow> w = std::make_unique<MainWindow>();
	w->show();
	int re = app.exec();
	QFontDatabase::removeAllApplicationFonts();

	// Explicitly delete main window because of dangling references
	w.reset(nullptr);

    // Destruct the app context singleton
    AppContext::destroy();

	return re;
}
