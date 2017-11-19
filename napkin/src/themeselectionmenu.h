#pragma once

#include <QMenu>
#include "actions.h"

class ThemeSelectionMenu : public QMenu {
Q_OBJECT
public:
    ThemeSelectionMenu();

    void refresh();

private:
    void onThemeChanged(const QString& theme);

    void checkCurrentTheme();

};
