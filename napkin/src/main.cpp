
#include "generic/filtertreeview.h"
#include "mainwindow.h"
#include "napkinresources.h"
#include "appcontext.h"

#include <QFontDatabase>
#include <utility/fileutils.h>

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

	MainWindow w;
	w.show();

	int re = app.exec();
	QFontDatabase::removeAllApplicationFonts();

    // Destruct the app context singleton
    AppContext::destroy();

	return re;
}
