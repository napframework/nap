/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
				outValue.setValue(QChar(value.to_int8()));
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
		nap::Logger::error("Could not convert to QVariant: %s", type.get_name().data());
		return false;
	}


	Variant fromQVariant(const TypeInfo& type, const QVariant& variant, bool* ok)
	{
		*ok = true;

		if (type.is_arithmetic())
		{
			if (type == TypeInfo::get<bool>())
				return variant.toBool();
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
			else if (type == TypeInfo::get<char>())
			{
				if (!variant.toString().isEmpty())
				{
					int8_t v = (int8_t)(variant.toString().data()[0].toLatin1());
					if (v >= 0)
						return v;
				}
				*ok = false;
				return 0;
			}
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
		std::string name("instanceProp_" + std::string(value.get_type().get_name().data()));
		auto propValue = doc->addObject<nap::TypedInstancePropertyValue<T>>(nullptr, name, false);
		propValue->mValue = value.get_value<T>();
		return propValue;
	}

	rttr::variant getInstancePropertyValue(rttr::type type, nap::InstancePropertyValue& instPropValue)
	{
		if (type == rttr::type::get<bool>()) 		return dynamic_cast<nap::TypedInstancePropertyValue<bool>&>(instPropValue).mValue;
		if (type == rttr::type::get<char>()) 		return dynamic_cast<nap::TypedInstancePropertyValue<char>&>(instPropValue).mValue;
		if (type == rttr::type::get<int8_t>()) 		return dynamic_cast<nap::TypedInstancePropertyValue<int8_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<int16_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<int16_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<int32_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<int32_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<int64_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<int64_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<uint8_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<uint8_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<uint16_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<uint16_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<uint32_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<uint32_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<uint64_t>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<uint64_t>&>(instPropValue).mValue;
		if (type == rttr::type::get<float>()) 		return dynamic_cast<nap::TypedInstancePropertyValue<float>&>(instPropValue).mValue;
		if (type == rttr::type::get<double>()) 		return dynamic_cast<nap::TypedInstancePropertyValue<double>&>(instPropValue).mValue;
		if (type == rttr::type::get<glm::vec2>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<glm::vec2>&>(instPropValue).mValue;
		if (type == rttr::type::get<glm::vec3>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<glm::vec3>&>(instPropValue).mValue;
		if (type == rttr::type::get<glm::vec4>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<glm::vec4>&>(instPropValue).mValue;
		if (type == rttr::type::get<glm::ivec2>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<glm::ivec2>&>(instPropValue).mValue;
		if (type == rttr::type::get<glm::ivec3>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<glm::ivec3>&>(instPropValue).mValue;
		if (type == rttr::type::get<glm::quat>()) 	return dynamic_cast<nap::TypedInstancePropertyValue<glm::quat>&>(instPropValue).mValue;
		if (type == rttr::type::get<std::string>()) return dynamic_cast<nap::TypedInstancePropertyValue<std::string>&>(instPropValue).mValue;

		return rttr::detail::get_invalid_type();
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

	template<typename T>
	void setInstProp(rttr::variant& prop, const rttr::variant& value)
	{
		auto propValue = prop.get_value<nap::TypedInstancePropertyValue<T>*>();
		propValue->mValue = value.get_value<T>();
	}

	void setInstancePropertyValue(rttr::variant& prop, const rttr::type& type, const rttr::variant& value)
	{
		if (type == rttr::type::get<bool>()) 				setInstProp<bool>(prop, value);
		else if (type == rttr::type::get<char>()) 			setInstProp<char>(prop, value);
		else if (type == rttr::type::get<int8_t>()) 		setInstProp<int8_t>(prop, value);
		else if (type == rttr::type::get<int16_t>()) 		setInstProp<int16_t>(prop, value);
		else if (type == rttr::type::get<int32_t>()) 		setInstProp<int32_t>(prop, value);
		else if (type == rttr::type::get<int64_t>()) 		setInstProp<int64_t>(prop, value);
		else if (type == rttr::type::get<uint8_t>()) 		setInstProp<uint8_t>(prop, value);
		else if (type == rttr::type::get<uint16_t>()) 		setInstProp<uint16_t>(prop, value);
		else if (type == rttr::type::get<uint32_t>()) 		setInstProp<uint32_t>(prop, value);
		else if (type == rttr::type::get<uint64_t>()) 		setInstProp<uint64_t>(prop, value);
		else if (type == rttr::type::get<float>()) 			setInstProp<float>(prop, value);
		else if (type == rttr::type::get<double>()) 		setInstProp<double>(prop, value);
		else if (type == rttr::type::get<glm::vec2>()) 		setInstProp<glm::vec2>(prop, value);
		else if (type == rttr::type::get<glm::vec3>()) 		setInstProp<glm::vec3>(prop, value);
		else if (type == rttr::type::get<glm::vec4>()) 		setInstProp<glm::vec4>(prop, value);
		else if (type == rttr::type::get<glm::ivec2>()) 	setInstProp<glm::ivec2>(prop, value);
		else if (type == rttr::type::get<glm::ivec3>()) 	setInstProp<glm::ivec3>(prop, value);
		else if (type == rttr::type::get<glm::quat>()) 		setInstProp<glm::quat>(prop, value);
		else if (type == rttr::type::get<std::string>()) 	setInstProp<std::string>(prop, value);
	}

	template<typename T>
	void removeInstProp(rttr::variant& prop)
	{
		auto doc = napkin::AppContext::get().getDocument();
		auto propValue = prop.get_value<nap::TypedInstancePropertyValue<T>*>();
		doc->removeObject(*propValue);
	}

	void removeInstancePropertyValue(rttr::variant& prop, const rttr::type& type)
	{
		if (type == rttr::type::get<bool>()) 				removeInstProp<bool>(prop);
		else if (type == rttr::type::get<char>()) 			removeInstProp<char>(prop);
		else if (type == rttr::type::get<int8_t>()) 		removeInstProp<int8_t>(prop);
		else if (type == rttr::type::get<int16_t>()) 		removeInstProp<int16_t>(prop);
		else if (type == rttr::type::get<int32_t>()) 		removeInstProp<int32_t>(prop);
		else if (type == rttr::type::get<int64_t>()) 		removeInstProp<int64_t>(prop);
		else if (type == rttr::type::get<uint8_t>()) 		removeInstProp<uint8_t>(prop);
		else if (type == rttr::type::get<uint16_t>()) 		removeInstProp<uint16_t>(prop);
		else if (type == rttr::type::get<uint32_t>()) 		removeInstProp<uint32_t>(prop);
		else if (type == rttr::type::get<uint64_t>()) 		removeInstProp<uint64_t>(prop);
		else if (type == rttr::type::get<float>()) 			removeInstProp<float>(prop);
		else if (type == rttr::type::get<double>()) 		removeInstProp<double>(prop);
		else if (type == rttr::type::get<glm::vec2>()) 		removeInstProp<glm::vec2>(prop);
		else if (type == rttr::type::get<glm::vec3>()) 		removeInstProp<glm::vec3>(prop);
		else if (type == rttr::type::get<glm::vec4>()) 		removeInstProp<glm::vec4>(prop);
		else if (type == rttr::type::get<glm::ivec2>()) 	removeInstProp<glm::ivec2>(prop);
		else if (type == rttr::type::get<glm::ivec3>()) 	removeInstProp<glm::ivec3>(prop);
		else if (type == rttr::type::get<glm::quat>()) 		removeInstProp<glm::quat>(prop);
		else if (type == rttr::type::get<std::string>()) 	removeInstProp<std::string>(prop);
	}
};
