#include <QFontDatabase>
#include <QCommandLineParser>
#include <QSplashScreen>

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
	// Start logging to file next to console
	nap::Logger::logToDirectory(nap::utility::getExecutableDir() + "/log", "napkin");

	// Only log debug messages and higher
	nap::Logger::setLevel(nap::Logger::debugLevel());

    // Construct the app context singleton
    auto& ctx = AppContext::create();

	// nap::Core is declared in AppContext
	QApplication::setOrganizationName("napframework");
	QApplication::setApplicationName("Napkin");
	QApplication::setApplicationVersion("0.4");

	initializeSettings();

	QApplication app(argc, argv);

	// Show splash screen
	QPixmap splashpic(QRC_ICONS_NAPKIN_SPLASH);
	QSplashScreen splash(splashpic);
	splash.show();
	app.processEvents();

	{
		// handle commandline
		QCommandLineParser parser;
		auto opHelp = parser.addHelpOption();
		auto opVer	= parser.addVersionOption();

		QCommandLineOption opProject({"p", "project"}, "Load specified project file upon startup", "project", "");
		parser.addOption(opProject);
		// Options to assist with automated testing
		QCommandLineOption opNoOpenRecent("no-project-reopen", "Don't attempt to re-open last project", "", "");
		parser.addOption(opNoOpenRecent);
		QCommandLineOption opExitFailure("exit-on-failure", "Exit on failure loading project (for testing)", "", "");
		parser.addOption(opExitFailure);
		QCommandLineOption opExitSuccess("exit-on-success", "Exit on success loading project (for testing)", "", "");
		parser.addOption(opExitSuccess);

		parser.process(app);

		if (parser.isSet(opHelp))
			return 0;

		if (parser.isSet(opVer))
			return 0;

		if (parser.isSet(opProject)) 
        {
            std::string projectPath = parser.value(opProject).toStdString();
		    projectPath = nap::utility::getAbsolutePath(projectPath);
			ctx.addRecentlyOpenedProject(QString::fromStdString(projectPath));
        } else if (parser.isSet(opNoOpenRecent))
			ctx.setOpenRecentProjectOnStartup(false);

		if (parser.isSet(opExitFailure))
			ctx.setExitOnLoadFailure(true);
		if (parser.isSet(opExitSuccess))
			ctx.setExitOnLoadSuccess(true);
	}

	// Create main window and run
	app.setWindowIcon(QIcon(QRC_ICONS_NAP_LOGO));
	std::unique_ptr<MainWindow> w = std::make_unique<MainWindow>();
	w->show();
	splash.finish(w.get());
	int re = app.exec();
	QFontDatabase::removeAllApplicationFonts();

	// Explicitly delete main window because of dangling references
	w.reset(nullptr);

    // Destruct the app context singleton
    AppContext::destroy();

	return re;
}
