#pragma once

#include <QString>

/**
 * It's kinda dirty but we do need some literals and this is the limbo they end up in (for now...).
 */
namespace napkin {
    // Display texts
    static const QString TXT_UNCONVERTIBLE_TYPE = "<<Could not convert to QVariant>>";
    static const QString TXT_LABEL_NAME = "Name";
    static const QString TXT_LABEL_VALUE = "Value";
    static const QString TXT_LABEL_TYPE = "Type";
    static const QString TXT_LABEL_OBJECTS = "Objects";
    static const QString TXT_LABEL_ENTITIES = "Entities";
    static const QString TXT_DEFAULT_THEME = "native";

    // The file filter used by file dialogs
    static const QString JSON_FILE_FILTER = "NAP JSON File (*.nap.json, *.json)";

    // Constants used by QSettings entries
    namespace settingsKey {
        static const QString WIN_GEO = "windowGeometry";
        static const QString WIN_STATE = "windowState";
        static const QString LAST_THEME = "lastTheme";
        static const QString LAST_OPENED_FILE = "lastOpenedFile";
    }

    static const std::string PROP_CHILDREN = "Children";
    static const std::string PROP_COMPONENTS = "Components";
}
