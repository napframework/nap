#include "typeconversion.h"
#include "appcontext.h"

#include <qdebug.h>

using namespace nap::rtti;

namespace napkin
{

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
			if (type == TypeInfo::get<bool>())
				outValue.setValue(value.to_bool());
			else if (type == TypeInfo::get<char>())
				outValue.setValue(value.to_int8());
			else if (type == TypeInfo::get<int8_t>())
				outValue.setValue(value.to_int8());
			else if (type == TypeInfo::get<int16_t>())
				outValue.setValue(value.to_int16());
			else if (type == TypeInfo::get<int32_t>())
				outValue.setValue(value.to_int32());
			else if (type == TypeInfo::get<int64_t>())
				outValue.setValue(value.to_int64());
			else if (type == TypeInfo::get<uint8_t>())
				outValue.setValue(value.to_int());
			else if (type == TypeInfo::get<uint16_t>())
				outValue.setValue(value.to_uint16());
			else if (type == TypeInfo::get<uint32_t>())
				outValue.setValue(value.to_uint32());
			else if (type == TypeInfo::get<uint64_t>())
				outValue.setValue(value.to_uint64());
			else if (type == TypeInfo::get<float>())
				outValue.setValue(value.to_float());
			else if (type == TypeInfo::get<double>())
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
		}
		else if (type == TypeInfo::get<std::string>())
		{
			outValue.setValue(QString::fromStdString(value.to_string()));
			return true;
		}

		return false;
	}


	Variant fromQVariant(const TypeInfo& type, const QVariant& variant, bool* ok)
	{
		*ok = true;

		if (type.is_arithmetic())
		{
			if (type == TypeInfo::get<bool>())
				return variant.toBool();
			else if (type == TypeInfo::get<char>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<int8_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<int16_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<int32_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<int64_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<uint8_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<uint16_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<uint32_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<uint64_t>())
				return variant.toInt(ok);
			else if (type == TypeInfo::get<float>())
				return variant.toFloat(ok);
			else if (type == TypeInfo::get<double>())
				return variant.toReal(ok);
		}
		else if (type.is_enumeration())
		{
			auto stringvalue = enumIndexToStdString(type.get_enumeration(), variant.toInt());
			if (stringvalue.length() == 0)
				*ok = false;
			return stringvalue;
		}
		else if (type == TypeInfo::get<std::string>())
		{
			return variant.toString().toStdString();
		}

		// Unknown type
		assert(false);
		return Variant();
	}

	template<typename T>
	nap::TypedInstancePropertyValue<T>* addInstProp(const rttr::variant& value)
	{
		auto doc = napkin::AppContext::get().getDocument();
		std::string name("instnceProp_" + std::string(value.get_type().get_name().data()));
		auto propValue = doc->addObject<nap::TypedInstancePropertyValue<T>>(nullptr, name, false);
		propValue->mValue = value.get_value<T>();
		return propValue;
	}

	nap::InstancePropertyValue* createInstancePropertyValue(const rttr::type& type, const rttr::variant& value)
	{
		if (type == rttr::type::get<bool>()) 		return addInstProp<bool>(value);
		if (type == rttr::type::get<char>()) 		return addInstProp<char>(value);
		if (type == rttr::type::get<int8_t>()) 		return addInstProp<int8_t>(value);
		if (type == rttr::type::get<int16_t>()) 	return addInstProp<int16_t>(value);
		if (type == rttr::type::get<int32_t>()) 	return addInstProp<int32_t>(value);
		if (type == rttr::type::get<int64_t>()) 	return addInstProp<int64_t>(value);
		if (type == rttr::type::get<uint8_t>()) 	return addInstProp<uint8_t>(value);
		if (type == rttr::type::get<uint16_t>()) 	return addInstProp<uint16_t>(value);
		if (type == rttr::type::get<uint32_t>()) 	return addInstProp<uint32_t>(value);
		if (type == rttr::type::get<uint64_t>()) 	return addInstProp<uint64_t>(value);
		if (type == rttr::type::get<float>()) 		return addInstProp<float>(value);
		if (type == rttr::type::get<double>()) 		return addInstProp<double>(value);
		if (type == rttr::type::get<glm::vec2>()) 	return addInstProp<glm::vec2>(value);
		if (type == rttr::type::get<glm::vec3>()) 	return addInstProp<glm::vec3>(value);
		if (type == rttr::type::get<glm::vec4>()) 	return addInstProp<glm::vec4>(value);
		if (type == rttr::type::get<glm::ivec2>()) 	return addInstProp<glm::ivec2>(value);
		if (type == rttr::type::get<glm::ivec3>()) 	return addInstProp<glm::ivec3>(value);
		if (type == rttr::type::get<glm::quat>()) 	return addInstProp<glm::quat>(value);
		if (type == rttr::type::get<std::string>()) return addInstProp<std::string>(value);

		nap::Logger::error("Instance property type '%s' not supported", type.get_name().data());

		return nullptr;
	}
};