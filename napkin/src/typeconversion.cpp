#include "typeconversion.h"

#include <qdebug.h>
using namespace nap;


uint64_t enumStringToIndex(rttr::enumeration enumer, const std::string& name, bool* ok)
{
	// TODO: Not sure how rttr::enumeration::name_to_value works, so doing it like this for now
	uint64_t i = 0;
	for (auto nm : enumer.get_names())
	{
		if (std::string(nm.data()) == name)
		{
			*ok = true;
			return i;
		}
		i++;
	}
	*ok = false;
	return 0;
}

QString enumIndexToQString(rttr::enumeration enumer, int index)
{
	return QString::fromStdString(enumIndexToStdString(enumer, index));
}


std::string enumIndexToStdString(rttr::enumeration enumer, int index)
{
	uint64_t i = 0;
	for (const auto& nm : enumer.get_names())
	{
		if (i == index)
		{
			return nm.data();
		}
		i++;
	}
	return std::string();
}


bool toQVariant(const nap::rtti::TypeInfo& type, const nap::rtti::Variant& value, QVariant& outValue)
{

	if (type.is_arithmetic())
	{
		if (type == rtti::TypeInfo::get<bool>())
			outValue.setValue(value.to_bool());
		else if (type == rtti::TypeInfo::get<char>())
			outValue.setValue(value.to_uint8());
		else if (type == rtti::TypeInfo::get<int8_t>())
			outValue.setValue(value.to_int8());
		else if (type == rtti::TypeInfo::get<int16_t>())
			outValue.setValue(value.to_int16());
		else if (type == rtti::TypeInfo::get<int32_t>())
			outValue.setValue(value.to_int32());
		else if (type == rtti::TypeInfo::get<int64_t>())
			outValue.setValue(value.to_int64());
		else if (type == rtti::TypeInfo::get<uint8_t>())
			outValue.setValue(value.to_uint8());
		else if (type == rtti::TypeInfo::get<uint16_t>())
			outValue.setValue(value.to_uint16());
		else if (type == rtti::TypeInfo::get<uint32_t>())
			outValue.setValue(value.to_uint32());
		else if (type == rtti::TypeInfo::get<uint64_t>())
			outValue.setValue(value.to_uint64());
		else if (type == rtti::TypeInfo::get<float>())
			outValue.setValue(value.to_float());
		else if (type == rtti::TypeInfo::get<double>())
			outValue.setValue(value.to_double());
		else
			return false;
		return true;
	}
	else if (type.is_enumeration())
	{

		// is string, convert to variant, then to int
		bool conversion_succeeded = false;
		std::string val = value.to_string(&conversion_succeeded);
		if (conversion_succeeded)
		{
			uint64_t value_int = enumStringToIndex(type.get_enumeration(), val, &conversion_succeeded);
			if (conversion_succeeded)
			{
				outValue.setValue(value_int);
				return true;
			}
			else
			{
				return false;
			}
		}

		// TODO: is this necessary?
		//        if (!conversion_succeeded)
		//            return false;
		//        } else {
		//            bool conversion_succeeded = false;
		//            value_int = value.to_uint64(&conversion_succeeded);
		//            if (!conversion_succeeded)
		//                return false;
		//        }
		//
		//        outValue.setValue(value_int);
	}
	else if (type == rtti::TypeInfo::get<std::string>())
	{
		outValue.setValue(QString::fromStdString(value.to_string()));
		return true;
	}

	return false;
}


rtti::Variant fromQVariant(const rtti::TypeInfo& type, const QVariant& variant, bool* ok)
{
	*ok = true;

	if (type.is_arithmetic())
	{
		if (type == rtti::TypeInfo::get<bool>())
			return variant.toBool();
		else if (type == rtti::TypeInfo::get<char>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<int8_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<int16_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<int32_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<int64_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<uint8_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<uint16_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<uint32_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<uint64_t>())
			return variant.toInt(ok);
		else if (type == rtti::TypeInfo::get<float>())
			return variant.toFloat(ok);
		else if (type == rtti::TypeInfo::get<double>())
			return variant.toReal(ok);
	}
	else if (type.is_enumeration())
	{
		auto stringvalue = enumIndexToStdString(type.get_enumeration(), variant.toInt());
		if (stringvalue.length() == 0)
			*ok = false;
		return stringvalue;
	}
	else if (type == rtti::TypeInfo::get<std::string>())
	{
		return variant.toString().toStdString();
	}

	// Unknown type
	assert(false);
	return rtti::Variant();
}
