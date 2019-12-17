#include <string>
#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rttr/type>
#include <rtti/deserializeresult.h>
#include <utility/errorstate.h>
#include <utility/fileutils.h>
#include "logger.h"

using namespace rapidjson;
using namespace rttr;

namespace nap
{
	void fromJSON(instance inst, Value& jsonObject);

	variant extractBasicTypes(Value& jsonValue)
	{
		switch (jsonValue.GetType())
		{
			case kStringType:
			{
				return std::string(jsonValue.GetString());
				break;
			}
			case kNullType:
				break;
			case kFalseType:
			case kTrueType:
			{
				return jsonValue.GetBool();
				break;
			}
			case kNumberType:
			{
				if (jsonValue.IsInt())
					return jsonValue.GetInt();
				else if (jsonValue.IsDouble())
					return jsonValue.GetDouble();
				else if (jsonValue.IsUint())
					return jsonValue.GetUint();
				else if (jsonValue.IsInt64())
					return jsonValue.GetInt64();
				else if (jsonValue.IsUint64())
					return jsonValue.GetUint64();
				break;
			}
				// we handle only the basic types here
			case kObjectType:
			case kArrayType:
				return variant();
		}

		return variant();
	}


	static void writeArray(variant_array_view& view, Value& jsonArrayValue)
	{
		view.set_size(jsonArrayValue.Size());
		const type array_value_type = view.get_rank_type(1);

		for (SizeType i = 0; i < jsonArrayValue.Size(); ++i)
		{
			auto& json_index_value = jsonArrayValue[i];
			if (json_index_value.IsArray())
			{
				auto sub_array_view = view.get_value(i).create_array_view();
				writeArray(sub_array_view, json_index_value);
			}
			else if (json_index_value.IsObject())
			{
				variant var_tmp = view.get_value(i);
				variant wrapped_var = var_tmp.extract_wrapped_value();
				fromJSON(wrapped_var, json_index_value);
				view.set_value(i, wrapped_var);
			}
			else
			{
				variant extracted_value = extractBasicTypes(json_index_value);
				if (extracted_value.convert(array_value_type))
					view.set_value(i, extracted_value);
			}
		}
	}

	variant extractValue(Value::MemberIterator& itr, const type& t)
	{
		auto& json_value = itr->value;
		variant extracted_value = extractBasicTypes(json_value);
		const bool could_convert = extracted_value.convert(t);
		if (!could_convert)
		{
			if (json_value.IsObject())
			{
				constructor ctor = t.get_constructor();
				for (auto& item : t.get_constructors())
				{
					if (item.get_instanciated_type() == t)
						ctor = item;
				}
				extracted_value = ctor.invoke();
				fromJSON(extracted_value, json_value);
			}
		}

		return extracted_value;
	}

	static void writeAssocView(variant_associative_view& view, Value& jsonArrayValue)
	{
		for (SizeType i = 0; i < jsonArrayValue.Size(); ++i)
		{
			auto& json_index_value = jsonArrayValue[i];
			if (json_index_value.IsObject()) // a key-value associative view
			{
				Value::MemberIterator key_itr = json_index_value.FindMember("key");
				Value::MemberIterator value_itr = json_index_value.FindMember("value");

				if (key_itr != json_index_value.MemberEnd() &&
					value_itr != json_index_value.MemberEnd())
				{
					auto key_var = extractValue(key_itr, view.get_key_type());
					auto value_var = extractValue(value_itr, view.get_value_type());
					if (key_var && value_var)
					{
						view.insert(key_var, value_var);
					}
				}
			}
			else // a key-only associative view
			{
				variant extracted_value = extractBasicTypes(json_index_value);
				if (extracted_value && extracted_value.convert(view.get_key_type()))
					view.insert(extracted_value);
			}
		}
	}

	void fromJSON(instance inst, Value& jsonObject)
	{
		instance obj = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;
		const auto prop_list = obj.get_derived_type().get_properties();

		for (auto prop : prop_list)
		{
			Value::MemberIterator ret = jsonObject.FindMember(prop.get_name().data());
			if (ret == jsonObject.MemberEnd())
				continue;
			const type value_t = prop.get_type();

			auto& json_value = ret->value;
			switch (json_value.GetType())
			{
				case kArrayType:
				{
					variant var;
					if (value_t.is_array())
					{
						var = prop.get_value(obj);
						auto view = var.create_array_view();
						writeArray(view, json_value);
					}
					else if (value_t.is_associative_container())
					{
						var = prop.get_value(obj);
						auto associative_view = var.create_associative_view();
						writeAssocView(associative_view, json_value);
					}

					prop.set_value(obj, var);
					break;
				}
				case kObjectType:
				{
					variant var = prop.get_value(obj);
					fromJSON(var, json_value);
					prop.set_value(obj, var);
					break;
				}
				default:
				{
					variant extracted_value = extractBasicTypes(json_value);
					// CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
					if (extracted_value.convert(value_t))
						prop.set_value(obj, extracted_value);
				}
			}
		}
	}

	bool deserializeSimple(const std::string& json, rttr::instance obj, nap::utility::ErrorState& err)
	{
		Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.

		// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
		if (document.Parse(json.c_str()).HasParseError())
		{
			err.fail(nap::utility::stringFormat("Failed to parse json: %s",
												rapidjson::GetParseError_En(document.GetParseError())));
			return false;
		}

		fromJSON(obj, document);

		return true;
	}

	bool loadJSONSimple(const std::string& filename, rttr::instance obj, nap::utility::ErrorState& err)
	{
		std::string buffer;
		if (!readFileToString(filename, buffer, err))
			return false;

		if (!nap::deserializeSimple(buffer, obj, err))
			return false;

		return true;
	}

}