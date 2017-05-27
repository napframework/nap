
#include "appcontext.h"

#include <rtti/rtti.h>

#ifdef _MSC_VER
//#include "module_napgui.cpp"
#include <ofMain.h>
#include "module_tommy.cpp"
#include "mainwindow.h"

class ofApp : public ofBaseApp
{
public:
	ofApp() {}

	void setup() 
	{
	}
	void update() {}
	void draw() {}
	void exit() {}

};

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	QApplication::setOrganizationName("NAP");
	QApplication::setApplicationName("Napkin");

	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1024, 768, OF_WINDOW);
	//ofRunApp(new ofApp());
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
