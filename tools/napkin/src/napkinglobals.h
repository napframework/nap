/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QString>
#include <QStandardItem>
#include <nap/core.h>

/**
 * It's kinda dirty but we do need some literals and this is the limbo they end up in (for now...).
 */
namespace napkin
{
	// Display texts
	inline constexpr const char* TXT_UNCONVERTIBLE_TYPE	= "<<Could not convert to QVariant>>";
	inline constexpr const char* TXT_LABEL_NAME			= "Name";
	inline constexpr const char* TXT_LABEL_VALUE		= "Value";
	inline constexpr const char* TXT_LABEL_TYPE			= "Type";
	inline constexpr const char* TXT_LABEL_RESOURCES	= "Resources";
	inline constexpr const char* TXT_LABEL_ENTITIES		= "Entities";
	inline constexpr const char* TXT_UNTITLED_DOCUMENT	= "Untitled Document";
	inline constexpr const char* TXT_THEME_DEFAULT		= "Dark";

	// The file filter used by file dialogs
	inline constexpr const char* JSON_FILE_EXT			= "json";
	inline constexpr const char* JSON_CONFIG_FILTER		= "NAP Config File (*.json)";
	inline constexpr const char* JSON_DATA_FILTER		= "NAP Data File (*.json)";
	inline constexpr const char* DEFAULT_SETTINGS_FILE	= "resources/defaultsettings.ini";
	inline constexpr int MAX_RECENT_FILES			    = 10;

	// Constants used by QSettings entries
	namespace settingsKey
	{
		inline constexpr const char* WIN_GEO			= "windowGeometry";
		inline constexpr const char* WIN_STATE			= "windowState";
		inline constexpr const char* LAST_THEME			= "lastTheme";
		inline constexpr const char* RECENTLY_OPENED	= "recentProjects";
	}

	inline constexpr const char* PROP_CHILDREN			= "Children";
	inline constexpr const char* PROP_COMPONENTS		= "Components";
	inline constexpr const char* NAP_URI_PREFIX			= "nap";

    // Testing success exit code. Avoids some common Unix error codes
	inline constexpr int EXIT_ON_SUCCESS_EXIT_CODE = 180;
}
