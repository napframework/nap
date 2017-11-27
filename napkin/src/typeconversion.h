#pragma once

#include <QVariant>
#include <rttr/type.h>
#include <mutex>
#include <rtti/typeinfo.h>

/**
 * This wrapper allows rttr types to be used in QVariant
 */
class TypeWrapper
{
public:
    TypeWrapper()
    {}

    TypeWrapper(rttr::type* t) : type(t)
    {}

    // The wrapped type.
    rttr::type* type;
};

Q_DECLARE_METATYPE(TypeWrapper)

/**
 * Attempt to convert an enum string value into its integer counterpart. 
 * @param enumer The enumeration to use while converting
 * @param name The name of the enum value
 * @param ok Will be set to true if all was dandy, false if it failed.
 * @return The enum value as an integer
 */
uint64_t enumStringToIndex(rttr::enumeration enumer, const std::string& name, bool* ok);

/**
 * Convert an integer enum value into a QString.  
 * @param enumer The enumation to use while converting.
 * @param index The enum value as integer.
 * @return The enum value as a QString.
 */
QString enumIndexToQString(rttr::enumeration enumer, int index);

/**
 * Convert an integer enum value into an std::string.  
 * @param enumer The enumation to use while converting.
 * @param index The enum value as integer.
 * @return The enum value as a std::string.
 */
std::string enumIndexToStdString(rttr::enumeration enumer, int index);

/**
 * Convert an rttr::variant into a QVariant. 
 * @param type The type of the value.
 * @param value The value to convert.
 * @param outValue The resulting QVariant
 * @return True if the conversion was successful, false otherwise.
 */
bool toQVariant(const nap::rtti::TypeInfo& type, const rttr::variant& value, QVariant& outValue);

/**
 * Convert a QVariant value into an rttr::variant.
 * @param type The type of the value.
 * @param variant The value to convert.
 * @param ok Will be set to true if conversion was successful, false otherwise.
 * @return The resulting value as an rttr::variant
 */
rttr::variant fromQVariant(const nap::rtti::TypeInfo& type, const QVariant& variant, bool* ok);
