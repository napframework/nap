#pragma once

#include <QVariant>
#include <rttr/type.h>
#include <mutex>
#include <rtti/typeinfo.h>


class TypeWrapper {
public:
    TypeWrapper() {}
    TypeWrapper(rttr::type* t) : type(t) {}
    rttr::type* type;
};

Q_DECLARE_METATYPE(TypeWrapper)

bool toQVariant(const nap::rtti::TypeInfo& type,
                const nap::rtti::Variant& value, QVariant& outValue);

nap::rtti::Variant fromQVariant(const nap::rtti::TypeInfo& type,
                                const QVariant& variant, bool* ok);
