#include <QFontDatabase>

#include <utility/fileutils.h>

#include "mainwindow.h"
#include "napkin-resources.h"
#include "appcontext.h"

using namespace napkin;

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
	QApplication::setOrganizationName("NaiviSoftware");
	QApplication::setApplicationName("Napkin");

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(QRC_ICONS_NAP_LOGO));

	// Create main window and run
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
