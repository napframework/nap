#pragma once

#include <QVariant>
#include <rttr/type.h>
#include <mutex>


class TypeConverter {
public:
    TypeConverter() = default;

    virtual rttr::type getType() = 0;

    virtual QVariant toVariant(const rttr::property& prop, const rttr::instance& inst) = 0;

    virtual bool fromVariant(const rttr::property& prop, rttr::instance& inst, const QVariant& value) = 0;

    static TypeConverter* get(const rttr::type& t);

    static std::vector<std::unique_ptr<TypeConverter>>& typeConverters();

};

template<typename T>
class TypeConverterImpl : public TypeConverter {
public:
    TypeConverterImpl() = default;

    rttr::type getType() override { return rttr::type::get<T>(); }

    QVariant toVariant(const rttr::property& prop, const rttr::instance& inst) override;

    bool fromVariant(const rttr::property& prop, rttr::instance& inst, const QVariant& value) override;
};

