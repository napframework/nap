
#include <QApplication>
#include "generic/basewindow.h"

int main(int argc, char* argv[]) {
    QApplication::setOrganizationName("NaiviSoftware");
    QApplication::setApplicationName("Napkin");

    QApplication app(argc, argv);

    BaseWindow w;
    w.show();

    return app.exec();
}


