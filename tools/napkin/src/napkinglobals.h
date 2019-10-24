#pragma once

#include <QString>
#include <QStandardItem>

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
	static const QString JSON_FILE_FILTER = QString("NAP JSON File (*.%1)").arg(JSON_FILE_EXT);

	static const QString DEFAULT_SETTINGS_FILE = "resources/defaultsettings.ini";

	static const int MAX_RECENT_FILES = 10;

	// Constants used by QSettings entries
	namespace settingsKey
	{
		static const QString WIN_GEO		   	= "windowGeometry";
		static const QString WIN_STATE		   	= "windowState";
		static const QString LAST_THEME		   	= "lastTheme";
		static const QString RECENTLY_OPENED	= "recentlyOpened";
	}

	static const std::string PROP_CHILDREN   = "Children";
	static const std::string PROP_COMPONENTS = "Components";

	static const std::string NAP_URI_PREFIX	 = "nap";

}
