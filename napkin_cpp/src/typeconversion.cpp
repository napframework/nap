#include "typeconversion.h"
#include "globals.h"

using namespace std;
using namespace rttr;
using namespace napkin;

const QVariant TypeConverter::invalid = TXT_UNCONVERTIBLE_TYPE; // NOLINT

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TO_Q_VARIANT(TYPE) \
    template<> QVariant TypeConverterImpl<TYPE>::toVariant(const rttr::property prop, const rttr::instance inst) const

#define FROM_Q_VARIANT(TYPE) \
    template<> bool TypeConverterImpl<TYPE>::fromVariant(const rttr::property prop, rttr::instance inst, const QVariant value) const

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// bool
TO_Q_VARIANT(bool) { return prop.get_value(inst).to_bool(); }

FROM_Q_VARIANT(bool) {
    prop.set_value(inst, value.toBool());
    return true;
}


// int
TO_Q_VARIANT(int) { return prop.get_value(inst).to_int(); }

FROM_Q_VARIANT(int) {
    bool ok;
    int v = value.toInt(&ok);
    if (!ok) return false;
    prop.set_value(inst, v);
    return true;
}

// unsigned int
TO_Q_VARIANT(unsigned int) { return prop.get_value(inst).to_uint32(); }

FROM_Q_VARIANT(unsigned int) {
    bool ok;
    int v = value.toUInt(&ok);
    if (!ok) return false;
    prop.set_value(inst, v);
    return true;
}

// float
TO_Q_VARIANT(float) { return prop.get_value(inst).to_float(); }

FROM_Q_VARIANT(float) {
    bool ok;
    float v = value.toFloat(&ok);
    if (!ok) return false;
    prop.set_value(inst, v);
    return true;
}

// string
TO_Q_VARIANT(std::string) { return QString::fromStdString(prop.get_value(inst).to_string()); }

FROM_Q_VARIANT(std::string) {
    prop.set_value(inst, value.toString().toStdString());
    return true;
}


TypeConverter* TypeConverter::get(const type& t) {
    for (auto& conv : typeConverters())
        if (conv->getType() == t)
            return conv.get();
    return nullptr;
}

std::vector<std::unique_ptr<TypeConverter>>& TypeConverter::typeConverters() {
    static std::vector<std::unique_ptr<TypeConverter>> list;
    if (list.size() == 0) {
        list.emplace_back(std::make_unique<TypeConverterImpl<std::string>>());
        list.emplace_back(std::make_unique<TypeConverterImpl<bool>>());
        list.emplace_back(std::make_unique<TypeConverterImpl<float>>());
        list.emplace_back(std::make_unique<TypeConverterImpl<int>>());
        list.emplace_back(std::make_unique<TypeConverterImpl<unsigned int>>());
    }
    return list;
}
