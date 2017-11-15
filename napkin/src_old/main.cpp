
#include "appcontext.h"

#include <rtti/rtti.h>

#ifdef _MSC_VER
//#include "module_napgui.cpp"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	QApplication::setOrganizationName("NAP");
	QApplication::setApplicationName("Napkin");

	MainWindow w;
	w.show();



	//AppContext::get().initialize();
	return app.exec();
}


#else

int main(int argc, char* argv[])
{
	AppContext::get().spawnWindow();
}
#endif
