/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#include "mainwindow.h"
#include "appcontext.h"
#include "napkin-resources.h"
#include "napkin-env.h"

#include <QFontDatabase>
#include <QCommandLineParser>
#include <QSplashScreen>
#include <utility/fileutils.h>
#include <napkinutils.h>

namespace napkin
{
	// Napkin return error codes
	namespace returncode
	{
		inline constexpr int success		= 0;		//< No error
		inline constexpr int parseError		= 1;		//< Parse error
		inline constexpr int coreError		= 2;		//< Core initialization error
		inline constexpr int documentError	= 3;		//< Document load error
	}
}


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

	auto user_settings_filename = QSettings().fileName();
	if (!QFileInfo::exists(user_settings_filename))
	{
		auto settingsDir = QFileInfo(user_settings_filename).dir();
		if (!settingsDir.exists())
		{
			settingsDir.mkpath(".");
		}

		auto default_settings_filename = QString("%1/%2").arg(exeDir, napkin::DEFAULT_SETTINGS_FILE);
		if (!QFileInfo::exists(default_settings_filename))
		{
			nap::Logger::error("File not found: %s", default_settings_filename.toStdString().c_str());
		}

		if (!QFile::copy(default_settings_filename, user_settings_filename))
		{
			nap::Logger::error("Failed to copy %s to %s", 
				default_settings_filename.toStdString().c_str(),
				user_settings_filename.toStdString().c_str());
		}
	}
}


/**
 * Command line handling. Exits application if requested or arguments are invalid.
 * @param app the main application
 * @param context the main application context
 * @return if process should exit after load
 */
bool parseCommandline(QApplication& app, napkin::AppContext& context)
{
	QCommandLineParser parser;
	auto op_help = parser.addHelpOption();
	auto op_ver = parser.addVersionOption();

	QCommandLineOption opProject({ "p", "project" }, "Load specified project file upon startup", "project", "");
	parser.addOption(opProject);

	// Options to assist with automated testing
	QCommandLineOption opNoOpenRecent("no-project-reopen", "Don't attempt to re-open last project", "", "");
	parser.addOption(opNoOpenRecent);

	QCommandLineOption opExitAfterLoad("exit-after-load", "Exit after loading project (for testing)", "", "");
	parser.addOption(opExitAfterLoad);
	parser.process(app);

	if (parser.isSet(op_help))
		exit(napkin::returncode::success);

	if (parser.isSet(op_ver))
		exit(napkin::returncode::success);

	// Check if we need to exit after load
	bool exit_after_load = parser.isSet(opExitAfterLoad);

	// Check if there's a project to load
	bool project_set = parser.isSet(opProject);

	// Check if we should open a recent project
	bool no_open_recent = parser.isSet(opNoOpenRecent);

	// Bail if exit-after-load flag is set but no project is specified
	if (exit_after_load && !project_set)
	{
		nap::Logger::error("exit-after-load requested without specifying project to load");
		exit(napkin::returncode::parseError);
	}

	// Check if we need to load a specific project
	// This flag has precedence over the 'open no recent' project flag if set simultaneously
	auto ctx = napkin::utility::Context::get();
	if (project_set)
	{
		std::string project_path = parser.value(opProject).toStdString();
		project_path = nap::utility::getAbsolutePath(project_path);
		context.addRecentlyOpenedProject(QString::fromStdString(project_path));
	}
	else if (no_open_recent)
	{
		context.setOpenRecentProjectOnStartup(false);
	}

	return exit_after_load;
}


/**
 * Initialize the application and spawn its window
 */
int main(int argc, char* argv[])
{
	// Force X11 when running Linux -> required because of NAP applets.
	// Applets are NAP applications that run inside NAPKIN and use SDL2 which defaults to X11 - not wayland.
	// X applications executed in a wayland session use XWayland for compatibility.
	// It is recommended that you use X11 instead of wayland until properly supported by SDL and NVIDIA.
#ifdef __linux__
	env::set(env::option::QT_QPA_PLATFORM, "xcb");
#endif

	// Start logging to file next to console
	nap::Logger::logToDirectory(nap::utility::getExecutableDir() + "/log", "napkin");

    // Construct the app context singleton
    auto& ctx = napkin::AppContext::create();

	// nap::Core is declared in AppContext
	QApplication::setOrganizationName("nap-labs");
	QApplication::setApplicationName("napkin");
	QApplication::setApplicationVersion("0.8");

	// Configure settings
	initializeSettings();

	// Create app
	QApplication app(argc, argv);

	// Show splash screen during initialization of engine
	QPixmap splashpic(napkin::QRC_ICONS_NAPKIN_SPLASH);
	splashpic.setDevicePixelRatio(app.devicePixelRatio());
	QSplashScreen splash(splashpic);
	splash.show();
	app.processEvents();

	// We know what project to load when running from a packaged application environment -> No need to explicitly select a project
	auto run_ctx = napkin::utility::Context::get();
	if (run_ctx.getType() == napkin::utility::Context::EType::Application)
	{
		QFileInfo project_info(run_ctx.getRoot(), PROJECT_INFO_FILENAME);
		if (project_info.exists())
			ctx.addRecentlyOpenedProject(project_info.absoluteFilePath());
	}

	// Handle command line instructions
	bool exit_after_load = parseCommandline(app, ctx);

	// Create main window and show -> loads a project if set
	// Disable font hinting -> doesn't do font rendering any good
	app.setWindowIcon(QIcon(napkin::QRC_ICONS_NAP_ICON));
	std::unique_ptr<napkin::MainWindow> w = std::make_unique<napkin::MainWindow>();
	QFont applied_font = w->font();
	applied_font.setStyleStrategy(QFont::PreferQuality);
	applied_font.setHintingPreference(QFont::PreferNoHinting);
	w->setFont(applied_font);
	w->show();
	splash.finish(w.get());


	// Initialize return code.
	// Informs the test environment if the NAP project loaded successfully.
	// Only relevant when 'exit-after-load' flag is set for testing purposes.
	int exit_code = !ctx.getCore().isInitialized() ? napkin::returncode::coreError :
		!ctx.hasDocument() ? napkin::returncode::documentError :
		napkin::returncode::success;

	// Run until quit, this is the normal behavior.
	if (!exit_after_load)
		exit_code = app.exec();

	// Remove all app fonts
	QFontDatabase::removeAllApplicationFonts();

	// Explicitly delete main window because of dangling references
	w.reset(nullptr);

    // Destruct the app context singleton
    napkin::AppContext::destroy();
	return exit_code;
}

