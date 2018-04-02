#pragma once

#include "actions.h"
#include <QMenu>

namespace napkin {

    /**
     * A menu containing the available themes. The current theme is checked.
     */
    class ThemeSelectionMenu : public QMenu
    {
    Q_OBJECT
    public:
        ThemeSelectionMenu();

        /**
         * Refresh the menu
         */
        void refresh();

    private:
        /**
         * Called when the theme has been changed
         * @param theme
         */
        void onThemeChanged(const Theme* theme);

        /**
         * Set the current theme as checked in the menu
         */
        void checkCurrentTheme();
    };

};