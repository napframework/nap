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
	static const QString TXT_UNCONVERTIBLE_TYPE = "<<Could not convert to QVariant>>";
	static const QString TXT_LABEL_NAME			= "Name";
	static const QString TXT_LABEL_VALUE		= "Value";
	static const QString TXT_LABEL_TYPE			= "Type";
	static const QString TXT_LABEL_RESOURCES	= "Resources";
	static const QString TXT_LABEL_ENTITIES		= "Entities";
	static const QString TXT_UNTITLED_DOCUMENT	= "Untitled Document";
	static const QString TXT_THEME_DEFAULT		= "Napkin";

	// The file filter used by file dialogs
	static const QString JSON_FILE_EXT = "json";
	static const QString JSON_CONFIG_FILTER = QString("NAP Config File (*.%1)").arg(JSON_FILE_EXT);
	static const QString JSON_DATA_FILTER = QString("NAP Data File (*.%1)").arg(JSON_FILE_EXT);
	static const QString JSON_PROJECT_FILTER = QString("NAP Project File (%1)").arg(PROJECT_INFO_FILENAME);

	static const QString DEFAULT_SETTINGS_FILE = "resources/defaultsettings.ini";

	static const int MAX_RECENT_FILES = 10;

	// Constants used by QSettings entries
	namespace settingsKey
	{
		static const QString WIN_GEO		   	= "windowGeometry";
		static const QString WIN_STATE		   	= "windowState";
		static const QString LAST_THEME		   	= "lastTheme";
		static const QString RECENTLY_OPENED	= "recentProjects";
	}

	static const std::string PROP_CHILDREN   = "Children";
	static const std::string PROP_COMPONENTS = "Components";

	static const std::string NAP_URI_PREFIX	 = "nap";

    // Testing success exit code. Avoids some common Unix error codes
	static const int EXIT_ON_SUCCESS_EXIT_CODE = 180;
}
