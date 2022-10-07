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
			{
				std::string str = value.to_string();
				outValue.setValue(str.empty() ? QChar(0) : QChar(str[0]));
			}
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
					return variant.toString().toStdString()[0];
				*ok = false;
				return QChar(0).toLatin1();
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


	rttr::variant getInstancePropertyValue(const nap::InstancePropertyValue& instPropValue)
	{
		nap::rtti::Property inst_prop = instPropValue.get_type().get_property(nap::rtti::instanceproperty::value);
		return inst_prop.get_value(instPropValue);
	}


	template<typename T>
	nap::InstancePropertyValue* addInstanceProperty(const rttr::variant& value)
	{
		auto doc = napkin::AppContext::get().getDocument();
		std::string name("instanceProp_" + std::string(value.get_type().get_name().data()));
		auto propValue = doc->addObject<nap::TypedInstancePropertyValue<T>>(nullptr, name);
		propValue->mValue = value.get_value<T>();
		return propValue;
	}


	nap::InstancePropertyValue* createInstanceProperty(const rttr::type& type, napkin::Document& document)
	{
		const static std::unordered_map<rttr::type, rttr::type> instancePropertyMap
		{
			{ RTTI_OF(bool),		RTTI_OF(nap::BoolInstancePropertyValue)		},
			{ RTTI_OF(char),		RTTI_OF(nap::CharInstancePropertyValue)		},
			{ RTTI_OF(int8_t),		RTTI_OF(nap::Int8InstancePropertyValue)		},
			{ RTTI_OF(int16_t),		RTTI_OF(nap::Int16InstancePropertyValue)	},
			{ RTTI_OF(int32_t),		RTTI_OF(nap::Int32InstancePropertyValue)	},
			{ RTTI_OF(int64_t),		RTTI_OF(nap::Int64InstancePropertyValue)	},
			{ RTTI_OF(uint8_t),		RTTI_OF(nap::UInt16InstancePropertyValue)	},
			{ RTTI_OF(uint16_t),	RTTI_OF(nap::UInt32InstancePropertyValue)	},
			{ RTTI_OF(uint32_t),	RTTI_OF(nap::UInt32InstancePropertyValue)	},
			{ RTTI_OF(uint64_t),	RTTI_OF(nap::UInt64InstancePropertyValue)	},
			{ RTTI_OF(float),		RTTI_OF(nap::FloatInstancePropertyValue)	},
			{ RTTI_OF(double),		RTTI_OF(nap::DoubleInstancePropertyValue)	},
			{ RTTI_OF(glm::vec2),	RTTI_OF(nap::Vec2InstancePropertyValue)		},
			{ RTTI_OF(glm::vec3),	RTTI_OF(nap::Vec3InstancePropertyValue)		},
			{ RTTI_OF(glm::vec4),	RTTI_OF(nap::Vec4InstancePropertyValue)		},
			{ RTTI_OF(glm::ivec2),	RTTI_OF(nap::IVec2InstancePropertyValue)	},
			{ RTTI_OF(glm::ivec3),	RTTI_OF(nap::IVec3InstancePropertyValue)	},
			{ RTTI_OF(glm::quat),	RTTI_OF(nap::QuatInstancePropertyValue)		},
			{ RTTI_OF(std::string),	RTTI_OF(nap::StringInstancePropertyValue)	}
		};

		// Get property override to create
		nap::rtti::TypeInfo instance_property_type = nap::rtti::TypeInfo::empty();
		if (!type.is_wrapper())
		{
			// Find value type
			auto it = instancePropertyMap.find(type);
			if (it != instancePropertyMap.end())
				instance_property_type = it->second;
		}
		else
		{
			// Pointer type
			if (type.get_wrapped_type().is_pointer())
				instance_property_type = RTTI_OF(nap::PointerInstancePropertyValue);
		}

		// Bail if invalid
		if (!instance_property_type.is_valid())
		{
			nap::Logger::error("Instance property type '%s' not supported", type.get_name().data());
			return nullptr;
		}

		// Create
		auto instance_property = document.addObject(instance_property_type, nullptr, "override");
		assert(instance_property != nullptr);
		return rtti_cast<nap::InstancePropertyValue>(instance_property);
	}


	bool setInstancePropertyValue(rttr::variant& instanceProperty, const rttr::variant& value)
	{
		auto instance_property = instanceProperty.get_value<nap::InstancePropertyValue*>();
		assert(instance_property != nullptr);
		nap::rtti::Property value_property = instance_property->get_type().get_property(nap::rtti::instanceproperty::value);
		return value_property.set_value(instance_property, value);
	}


	void removeInstanceProperty(rttr::variant& instanceProperty, napkin::Document& document)
	{
		auto prop_value = instanceProperty.get_value<nap::InstancePropertyValue*>();
		assert(prop_value != nullptr);
		document.removeObject(*prop_value);
	}
};
