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


uint64_t enumStringToIndex(rttr::enumeration enumer, const std::string& name, bool* ok);
QString enumIndexToQString(rttr::enumeration enumer, int index);
std::string enumIndexToStdString(rttr::enumeration enumer, int index);

bool toQVariant(const nap::rtti::TypeInfo& type,
                const nap::rtti::Variant& value, QVariant& outValue);

nap::rtti::Variant fromQVariant(const nap::rtti::TypeInfo& type,
                                const QVariant& variant, bool* ok);
