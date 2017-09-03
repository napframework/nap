#pragma once

#include <QVariant>
#include <rttr/type.h>
#include <mutex>


class TypeConverter {
public:
    TypeConverter() = default;

    virtual rttr::type getType() = 0;

    virtual QVariant toVariant(const rttr::property prop, const rttr::instance inst) const = 0;

    virtual bool fromVariant(const rttr::property prop, rttr::instance inst, const QVariant value) const = 0;

    static TypeConverter* get(const rttr::type& t);

    static std::vector<std::unique_ptr<TypeConverter>>& typeConverters();

    static QVariant toQVariant(const rttr::property prop, rttr::instance inst) {
        auto converter = get(prop.get_type());
        if (converter)
            return converter->toVariant(prop, inst);
        return invalid;
    }

    static bool fromQVariant(const rttr::property prop, rttr::instance inst, const QVariant value) {
        auto converter = get(prop.get_type());
        if (!converter)
            return false;
        return converter->fromVariant(prop, inst, value);
    }

private:
    static const QVariant invalid;
};


template<typename T>
class TypeConverterImpl : public TypeConverter {
public:
    TypeConverterImpl() = default;

    rttr::type getType() override { return rttr::type::get<T>(); }

    QVariant toVariant(const rttr::property prop, const rttr::instance inst) const override;

    bool fromVariant(const rttr::property prop, rttr::instance inst, const QVariant value) const override;
};

