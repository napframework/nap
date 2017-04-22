#include "JSonReader.h"
#include "resource.h"	// TODO: for initresult, perhaps move to another file

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	UnresolvedPointer::UnresolvedPointer(Object* object, const RTTI::Property& property, const std::string& targetID) :
		mObject(object),
		mProperty(property),
		mTargetID(targetID)
	{
	}


	static bool readArrayRecursively(RTTI::VariantArray& array, const rapidjson::Value& jsonArray, UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, InitResult& initResult);


	static RTTI::Variant readBasicType(const rapidjson::Value& json_value)
	{
		switch (json_value.GetType())
		{
		case rapidjson::kStringType:
		{
			return std::string(json_value.GetString());
			break;
		}
		case rapidjson::kNullType:
			break;
		case rapidjson::kFalseType:
		case rapidjson::kTrueType:
		{
			return json_value.GetBool();
			break;
		}
		case rapidjson::kNumberType:
		{
			if (json_value.IsInt())
				return json_value.GetInt();
			else if (json_value.IsDouble())
				return json_value.GetDouble();
			else if (json_value.IsUint())
				return json_value.GetUint();
			else if (json_value.IsInt64())
				return json_value.GetInt64();
			else if (json_value.IsUint64())
				return json_value.GetUint64();
			break;
		}
		}

		return RTTI::Variant();
	}


	static bool readObjectRecursive(RTTI::Instance object, const rapidjson::Value& jsonObject, UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, InitResult& initResult)
	{
		for (const RTTI::Property& property : object.get_derived_type().get_properties())
		{
			bool is_required = property.get_metadata(RTTI::EPropertyMetaData::Required).is_valid();
			bool is_file_link = property.get_metadata(RTTI::EPropertyMetaData::FileLink).is_valid();

			rapidjson::Value::ConstMemberIterator json_property = jsonObject.FindMember(property.get_name().data());
			if (json_property == jsonObject.MemberEnd())
			{
				if (!initResult.check(!is_required, "Required property %s not found in object of type %s", property.get_name().data(), object.get_derived_type().get_name().data()))
					return false;
				continue;
			}

			const RTTI::TypeInfo value_type = property.get_type();
			const rapidjson::Value& json_value = json_property->value;

			if (!initResult.check((is_file_link && value_type.get_raw_type().is_derived_from<std::string>()) || !is_file_link, "Encountered a non-string file link. This is not supported"))
				return false;

			if (value_type.is_pointer())
			{
				if (!initResult.check(value_type.get_raw_type().is_derived_from<nap::Object>(), "Encountered pointer to non-Object. This is not supported"))
					return false;

				if (!initResult.check(json_value.GetType() == rapidjson::kStringType, "Encountered pointer property of unknown type"))
					return false;

				std::string target = std::string(json_value.GetString());

				if (!initResult.check((is_required && !target.empty()) || (!is_required), "Required property %s not found in object of type %s", property.get_name().data(), object.get_derived_type().get_name().data()))
					return false;

				unresolvedPointers.push_back(UnresolvedPointer(object.try_convert<Object>(), property, target));
			}
			else
			{
				switch (json_value.GetType())
				{
				case rapidjson::kArrayType:
				{
					RTTI::Variant value;
					if (value_type.is_array())
					{
						value = property.get_value(object);
						auto array_view = value.create_array_view();
						if (!readArrayRecursively(array_view, json_value, unresolvedPointers, linkedFiles, initResult))
							return false;
					}
					else if (value_type.is_associative_container())
					{
						initResult.mErrorString = "Encountered currently unsupported associative property";
						return false;
					}

					property.set_value(object, value);
					break;
				}
				case rapidjson::kObjectType:
				{
					RTTI::Variant var = property.get_value(object);
					if (!readObjectRecursive(var, json_value, unresolvedPointers, linkedFiles, initResult))
						return false;
					property.set_value(object, var);
					break;
				}
				default:
				{
					RTTI::Variant extracted_value = readBasicType(json_value);
					if (extracted_value.convert(value_type))
						property.set_value(object, extracted_value);
				}
				}
			}

			if (is_file_link)
			{
				FileLink file_link;
				file_link.mSourceObjectID = object.try_convert<Object>()->mID;
				file_link.mTargetFile = property.get_value(object).get_value<std::string>();;
				linkedFiles.push_back(file_link);
			}
		}

		return true;
	}


	static bool readArrayRecursively(RTTI::VariantArray& array, const rapidjson::Value& jsonArray, UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, InitResult& initResult)
	{
		array.set_size(jsonArray.Size());

		const RTTI::TypeInfo array_type = array.get_rank_type(array.get_rank());

		if (!initResult.check(!array_type.is_pointer(), "Arrays of pointers are not supported yet"))
			return false;

		for (std::size_t i = 0; i < jsonArray.Size(); ++i)
		{
			const rapidjson::Value& json_element = jsonArray[i];
			if (json_element.IsArray())
			{
				RTTI::VariantArray sub_array = array.get_value_as_ref(i).create_array_view();
				if (!readArrayRecursively(sub_array, json_element, unresolvedPointers, linkedFiles, initResult))
					return false;
			}
			else if (json_element.IsObject())
			{
				RTTI::Variant var_tmp = array.get_value_as_ref(i);
				RTTI::Variant wrapped_var = var_tmp.extract_wrapped_value();
				if (!readObjectRecursive(wrapped_var, json_element, unresolvedPointers, linkedFiles, initResult))
					return false;
				array.set_value(i, wrapped_var);
			}
			else
			{
				RTTI::Variant extracted_value = readBasicType(json_element);
				if (extracted_value.convert(array_type))
					array.set_value(i, extracted_value);
			}
		}

		return true;
	}

	
	bool readJSonFile(const std::string& filename, ObjectList& readObjects, std::vector<FileLink>& linkedFiles, UnresolvedPointerList& unresolvedPointers, nap::InitResult& initResult)
	{
		std::ifstream in(filename, std::ios::in | std::ios::binary);
		if (!initResult.check(in.good(), "Unable to open file %s", filename.c_str()))
			return false;

		// Create buffer of appropriate size
		in.seekg(0, std::ios::end);
		size_t len = in.tellg();
		std::string buffer;
		buffer.resize(len);

		// Read all data
		in.seekg(0, std::ios::beg);
		in.read(&buffer[0], len);
		in.close();

		rapidjson::Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
		rapidjson::ParseResult parse_result = document.Parse(buffer.c_str());
		if (!parse_result)
		{
			size_t start = buffer.rfind('\n', parse_result.Offset());
			size_t end = buffer.find('\n', parse_result.Offset());

			if (start == std::string::npos)
				start = 0;
			else
				start += 1;

			if (end == std::string::npos)
				end = buffer.size();
			else
				end -= 1;

			std::string error_line = buffer.substr(start, end - start);

			initResult.mErrorString = nap::stringFormat("Error parsing %s: %s (line: %s)", filename.c_str(), rapidjson::GetParseError_En(parse_result.Code()), error_line.c_str());
			return false;
		}

		bool success = true;
		for (auto& object_pos = document.MemberBegin(); object_pos < document.MemberEnd(); ++object_pos)
		{
			const char* typeName = object_pos->name.GetString();
			RTTI::TypeInfo type_info = RTTI::TypeInfo::get_by_name(typeName);
			if (!initResult.check(type_info.is_valid(), "Unknown object type %s encountered.", typeName))
			{
				success = false;
				break;
			}

			if (!initResult.check(type_info.can_create_instance(), "Unable to instantiate object of type %s.", typeName))
			{
				success = false;
				break;
			}

			if (!initResult.check(type_info.is_derived_from(RTTI_OF(nap::Object)), "Unable to instantiate object %s. Class is not derived from Object.", typeName))
			{
				success = false;
				break;
			}

			Object* object = type_info.create<Object>();
			readObjects.push_back(object);

			if (!readObjectRecursive(*object, object_pos->value, unresolvedPointers, linkedFiles, initResult))
			{
				success = false;
				break;
			}
		}

		if (!success)
		{
			for (Object* object : readObjects)
			{
				delete object;
			}

			readObjects.clear();
			unresolvedPointers.clear();
		}

		return success;
	}

}
