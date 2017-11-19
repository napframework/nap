#pragma once

#include <QString>

namespace napkin {
    static const QString TXT_UNCONVERTIBLE_TYPE = "<<Could not convert to QVariant>>";
    static const QString TXT_LABEL_NAME = "Name";
    static const QString TXT_LABEL_VALUE = "Value";
    static const QString TXT_LABEL_TYPE = "Type";

    static const QString TXT_LABEL_OBJECTS = "Objects";
    static const QString TXT_LABEL_ENTITIES = "Entities";

    static const QString TXT_DEFAULT_THEME = "native";

    static const QString JSON_FILE_FILTER = "NAP JSON File (*.nap.json, *.json)";

    namespace settingsKey {
        static const QString WIN_GEO = "windowGeometry";
        static const QString WIN_STATE = "windowState";
        static const QString LAST_THEME = "lastTheme";
        static const QString LAST_OPENED_FILE = "lastOpenedFile";
    }

    static const std::string PROP_CHILDLREN = "Children";
    static const std::string PROP_COMPONENTS = "Components";
}
