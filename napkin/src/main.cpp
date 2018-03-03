
#include "generic/filtertreeview.h"
#include "mainwindow.h"
#include "napkinresources.h"

#include <QFontDatabase>

using namespace napkin;

/**
 * Initialize the application and spawn its window
 */
int main(int argc, char* argv[])
{
	// nap::Core is declared in AppContext
	QApplication::setOrganizationName("NaiviSoftware");
	QApplication::setApplicationName("Napkin");

	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(QRC_ICONS_NAP_LOGO));

	MainWindow w;
	w.show();

	int re = app.exec();
	QFontDatabase::removeAllApplicationFonts();

	return re;
}
