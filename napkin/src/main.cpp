
#include "generic/filtertreeview.h"
#include "mainwindow.h"

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

	// nap::Core is declared in AppContext
	QApplication::setOrganizationName("NaiviSoftware");
	QApplication::setApplicationName("Napkin");

	QApplication app(argc, argv);

	MainWindow w;
	w.show();

	int re = app.exec();
	QFontDatabase::removeAllApplicationFonts();

	return re;
}
