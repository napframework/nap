#pragma once

#include "actions.h"
#include <QMenu>

namespace napkin {

    class ThemeSelectionMenu : public QMenu
    {
    Q_OBJECT
    public:
        ThemeSelectionMenu();

        void refresh();

    private:
        void onThemeChanged(const QString& theme);

        void checkCurrentTheme();
    };

};