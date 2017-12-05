#pragma once

#include <QtGui/QIcon>
#include <rtti/rttiobject.h>
#include <QtCore/QMap>

namespace napkin {

    /**
     * Conveniently retrieve icons by object type
     */
    class ResourceFactory {

    public:
        ResourceFactory();

        QIcon iconFor(const nap::rtti::RTTIObject& object) const;
    private:
        QMap<rttr::type, QString> mObjectIconMap;
    };
}
