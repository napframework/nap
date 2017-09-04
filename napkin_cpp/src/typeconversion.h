#pragma once

#include <QVariant>
#include <rttr/type.h>
#include <mutex>
#include <rtti/typeinfo.h>


bool toQVariant(const nap::rtti::TypeInfo& type,
                const nap::rtti::Variant& value, QVariant& outValue);

nap::rtti::Variant fromQVariant(const nap::rtti::TypeInfo& type,
                                const QVariant& variant, bool* ok);
