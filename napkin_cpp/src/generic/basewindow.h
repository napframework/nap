#include <QtWidgets/QMainWindow>

class BaseWindow : public QMainWindow {
public:
    BaseWindow() : QMainWindow() {
        setDockNestingEnabled(true);

    }

    void addDock(const QString& name, QWidget* widget) {

    }

private:
};