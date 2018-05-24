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
	static const QString TXT_THEME_NATIVE		= "Native";

	// The file filter used by file dialogs
	static const QString JSON_FILE_FILTER = "NAP JSON File (*.nap.json, *.json)";

	static const QString DEFAULT_SETTINGS_FILE = "resources/settings.ini";

	// Constants used by QSettings entries
	namespace settingsKey
	{
		static const QString WIN_GEO		  = "windowGeometry";
		static const QString WIN_STATE		  = "windowState";
		static const QString LAST_THEME		  = "lastTheme";
		static const QString LAST_OPENED_FILE = "lastOpenedFile";
	}

	static const std::string PROP_CHILDREN   = "Children";
	static const std::string PROP_COMPONENTS = "Components";

	static const std::string NAP_URI_PREFIX	 = "nap";

    /**
     * Needed for Qt style QStandardItem casting
     */
	enum StandardItemTypeID
	{
        // Generic items
        EmptyID             = QStandardItem::UserType + 0,
        InvalidID           = QStandardItem::UserType + 1,

		// Object items
		GroupItemID		    = QStandardItem::UserType + 20,
		ObjectItemID	    = QStandardItem::UserType + 21,
		EntityItemID	    = QStandardItem::UserType + 22,
		ComponentItemID	    = QStandardItem::UserType + 23,
        SceneItemID         = QStandardItem::UserType + 24,
        EntityInstanceID    = QStandardItem::UserType + 25,

        // RTTIPath items
		RTTIPathID		    = QStandardItem::UserType + 50,
		PropertyID		    = QStandardItem::UserType + 51,
		PropertyValueID	    = QStandardItem::UserType + 52,
		EmbeddedPointerID   = QStandardItem::UserType + 53,
		CompoundPropertyID  = QStandardItem::UserType + 54,
		ArrayPropertyID	    = QStandardItem::UserType + 55,
		PointerID		    = QStandardItem::UserType + 56,
		PointerValueID	    = QStandardItem::UserType + 57,
	};
}
