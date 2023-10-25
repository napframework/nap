/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
