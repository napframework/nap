
#include <QApplication>
#include "generic/filtertreeview.h"
#include "mainwindow.h"

/**
 * Initialize the application and spawn its window
 */
int main(int argc, char* argv[])
{

    // nap::Core is declared in AppContext

    QApplication::setOrganizationName("NaiviSoftware");
    QApplication::setApplicationName("Napkin");

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}


