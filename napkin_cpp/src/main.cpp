
#include <QApplication>
#include <src/generic/filtertreeview.h>
#include "generic/basewindow.h"
#include "outlinepanel.h"




int main(int argc, char* argv[]) {
    QApplication::setOrganizationName("NaiviSoftware");
    QApplication::setApplicationName("Napkin");

    QApplication app(argc, argv);

    BaseWindow w;


    OutlinePanel outline;
    w.addDock("Outline", &outline);

    w.show();

    return app.exec();
}


