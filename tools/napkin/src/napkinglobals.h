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
	inline constexpr char* TXT_UNCONVERTIBLE_TYPE	= "<<Could not convert to QVariant>>";
	inline constexpr char* TXT_LABEL_NAME			= "Name";
	inline constexpr char* TXT_LABEL_VALUE			= "Value";
	inline constexpr char* TXT_LABEL_TYPE			= "Type";
	inline constexpr char* TXT_LABEL_RESOURCES		= "Resources";
	inline constexpr char* TXT_LABEL_ENTITIES		= "Entities";
	inline constexpr char* TXT_UNTITLED_DOCUMENT	= "Untitled Document";
	inline constexpr char* TXT_THEME_DEFAULT		= "Napkin";

	// The file filter used by file dialogs
	inline constexpr char* JSON_FILE_EXT			= "json";
	inline constexpr char* JSON_CONFIG_FILTER		= "NAP Config File (*.json)";
	inline constexpr char* JSON_DATA_FILTER			= "NAP Data File (*.json)";
	inline constexpr char* JSON_PROJECT_FILTER		= "NAP Project File (*.json)";

	inline constexpr char* DEFAULT_SETTINGS_FILE	= "resources/defaultsettings.ini";
	inline constexpr int MAX_RECENT_FILES			= 10;

	// Constants used by QSettings entries
	namespace settingsKey
	{
		inline constexpr char* WIN_GEO				= "windowGeometry";
		inline constexpr char* WIN_STATE			= "windowState";
		inline constexpr char* LAST_THEME			= "lastTheme";
		inline constexpr char* RECENTLY_OPENED		= "recentProjects";
	}

	inline constexpr char* PROP_CHILDREN			= "Children";
	inline constexpr char* PROP_COMPONENTS			= "Components";
	inline constexpr char* NAP_URI_PREFIX			= "nap";

    // Testing success exit code. Avoids some common Unix error codes
	inline constexpr int EXIT_ON_SUCCESS_EXIT_CODE = 180;
}
