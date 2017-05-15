#include "jsonreader.h"
#include "nap/errorstate.h"
#include "nap/object.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	static bool readArrayRecursively(nap::Object* rootObject, const RTTI::Property& property, RTTI::VariantArray& array, const rapidjson::Value& jsonArray, RTTI::RTTIPath& rttiPath,
		OwnedObjectList& readObjects, UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, ErrorState& errorState);

	static nap::Object* readObjectRecursive(const rapidjson::Value::ConstMemberIterator& jsonObject, OwnedObjectList& readObjects, UnresolvedPointerList& unresolvedPointers,
		std::vector<FileLink>& linkedFiles, nap::ErrorState& errorState);

	/**
	 * Helper function to read a basic JSON type to a C++ type
	 */
	static RTTI::Variant readBasicType(const rapidjson::Value& jsonValue)
	{
		switch (jsonValue.GetType())
		{
			case rapidjson::kStringType:
			{
				return std::string(jsonValue.GetString());
				break;
			}
			case rapidjson::kNullType:
				break;
			case rapidjson::kFalseType:
			case rapidjson::kTrueType:
			{
				return jsonValue.GetBool();
				break;
			}
			case rapidjson::kNumberType:
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
		}

		// Unknown type
		assert(false);
		return RTTI::Variant();
	}


	static bool readEmbeddedObject(const rapidjson::Value& jsonValue, OwnedObjectList& readObjects,
		UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, nap::Object*& resultObject, ErrorState& errorState)
	{
		resultObject = nullptr;

		if (jsonValue.GetType() == rapidjson::kStringType)
		{
			if (!errorState.check(std::string(jsonValue.GetString()).empty(), "Encountered embedded pointer that points to a non-embedded object"))
				return false;
		}
		else
		{
			// Must be exactly one member in the case of an embedded pointer
			if (!errorState.check(jsonValue.MemberCount() == 1, "Encountered an embedded pointer which has more than 1 child"))
				return false;

			// Deserialize the nested object
			resultObject = readObjectRecursive(jsonValue.MemberBegin(), readObjects, unresolvedPointers, linkedFiles, errorState);
			if (resultObject == nullptr)
				return false;
		}

		return true;
	}

	/**
	 * Helper function to recursively read an object (can be a nap::Object, nested compound or any other type) from JSON
	 */
	static bool readPropertiesRecursive(nap::Object* object, RTTI::Instance compound, const rapidjson::Value& jsonCompound, RTTI::RTTIPath& rttiPath, OwnedObjectList& readObjects, 
		UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, ErrorState& errorState)
	{
		// Determine the object type. Note that we want to *most derived type* of the object.
		RTTI::TypeInfo object_type = compound.get_derived_type();

		// Go through all properties of the object
		for (const RTTI::Property& property : object_type.get_properties())
		{
			// Push attribute on path
			rttiPath.pushAttribute(property.get_name().data());

			// Determine meta-data for the property
			bool is_required = RTTI::hasFlag(property, RTTI::EPropertyMetaData::Required);
			bool is_file_link = RTTI::hasFlag(property, RTTI::EPropertyMetaData::FileLink);

			// Check whether the property is present in the JSON. If it's not, but the property is required, throw an error
			rapidjson::Value::ConstMemberIterator json_property = jsonCompound.FindMember(property.get_name().data());
			if (json_property == jsonCompound.MemberEnd())
			{
				if (!errorState.check(!is_required, "Required property %s not found in object of type %s", property.get_name().data(), object_type.get_name().data()))
					return false;

				rttiPath.popBack();
				continue;
			}

			const RTTI::TypeInfo value_type = property.get_type();
			const rapidjson::Value& json_value = json_property->value;

			// If this is a file link, make sure it's of the expected type (string)
			if (!errorState.check((is_file_link && value_type.get_raw_type().is_derived_from<std::string>()) || !is_file_link, "Encountered a non-string file link. This is not supported"))
				return false;

			// If the property is of pointer type, we can't set the property here but need to resolve it later
			if (value_type.is_pointer())
			{
				// Pointer types must point to objects derived from nap::Object
				if (!errorState.check(value_type.get_raw_type().is_derived_from<nap::Object>(), "Encountered pointer to non-Object. This is not supported"))
					return false;

				bool is_embedded_pointer = RTTI::hasFlag(property, RTTI::EPropertyMetaData::Embedded);

				// Pointer types must be of string type or nested object in JSON
				if (!errorState.check(json_value.GetType() == rapidjson::kStringType || json_value.GetType() == rapidjson::kObjectType, "Encountered pointer property of unknown type"))
					return false;

				if (is_embedded_pointer)
				{
					nap::Object* target = nullptr;
					if (!readEmbeddedObject(json_value, readObjects, unresolvedPointers, linkedFiles, target, errorState))
						return false;
					
					property.set_value(compound, target);
				}
				else
				{
					// Determine the target of the pointer
					std::string target = std::string(json_value.GetString());

					// If the target is empty (i.e. null pointer), but the property is required, throw an error
					if (!errorState.check((is_required && !target.empty()) || (!is_required), "Required property %s not found in object of type %s", property.get_name().data(), object_type.get_name().data()))
						return false;

					// Add to list of unresolved pointers
					if (!target.empty())
						unresolvedPointers.push_back(UnresolvedPointer(object, rttiPath, target));
				}
			}
			else
			{
				// Regular property; read the value and set the property
				switch (json_value.GetType())
				{
					case rapidjson::kArrayType:
					{
						// If this is an array, read its elements recursively again (in case of nested compounds)
						RTTI::Variant value;
						if (value_type.is_array())
						{
							// Get instance of the current value (this is a copy) and create an array view on it so we can fill it
							value = property.get_value(compound);
							RTTI::VariantArray array_view = value.create_array_view();

							// Now read the array recursively into array view
							if (!readArrayRecursively(object, property, array_view, json_value, rttiPath, readObjects, unresolvedPointers, linkedFiles, errorState))
								return false;
						}
						else if (value_type.is_associative_container())
						{
							// Maps not supported (yet)
							errorState.fail("Encountered currently unsupported associative property");
							return false;
						}

						// Now copy the read array back into the target object
						property.set_value(compound, value);
						break;
					}
					case rapidjson::kObjectType:
					{
						// If the property is a nested compound, read it recursively
						RTTI::Variant var = property.get_value(compound);
						if (!readPropertiesRecursive(object, var, json_value, rttiPath, readObjects, unresolvedPointers, linkedFiles, errorState))
							return false;

						// Copy read object back into the target object
						property.set_value(compound, var);
						break;
					}
					default:
					{
						// Basic JSON type, read value and copy to target
						RTTI::Variant extracted_value = readBasicType(json_value);
						if (extracted_value.convert(value_type))
							property.set_value(compound, extracted_value);
					}
				}
			}

			// If this property is a file link, add it to the list of file links
			if (is_file_link)
			{
				FileLink file_link;
				file_link.mSourceObjectID	= compound.try_convert<Object>()->mID;
				file_link.mTargetFile		= property.get_value(compound).get_value<std::string>();;
				linkedFiles.push_back(file_link);
			}

			rttiPath.popBack();
		}

		return true;
	}


	/**
	 * Helper function to recursively read an array (can be an array of basic types, nested compound, or any other type) from JSON
	 */
	static bool readArrayRecursively(nap::Object* rootObject, const RTTI::Property& property, RTTI::VariantArray& array, const rapidjson::Value& jsonArray, RTTI::RTTIPath& rttiPath, 
		OwnedObjectList& readObjects, UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, ErrorState& errorState)
	{
		// Pre-size the array to avoid too many dynamic allocs
		array.set_size(jsonArray.Size());

		// Determine the rank of the array (i.e. how many dimensions it has)
		const RTTI::TypeInfo array_type = array.get_rank_type(array.get_rank());

		// Read values from JSON array
		for (std::size_t index = 0; index < jsonArray.Size(); ++index)
		{
			// Add array element to rtti path
			rttiPath.pushArrayElement(index);

			const rapidjson::Value& json_element = jsonArray[index];

			if (array_type.is_pointer())
			{
				// Pointer types must point to objects derived from nap::Object
				if (!errorState.check(array_type.get_raw_type().is_derived_from<nap::Object>(), "Encountered pointer to non-Object. This is not supported"))
					return false;

				bool is_embedded_pointer = RTTI::hasFlag(property, RTTI::EPropertyMetaData::Embedded);

				// Pointer types must be of string type or nested object in JSON
				if (!errorState.check(json_element.GetType() == rapidjson::kStringType || json_element.GetType() == rapidjson::kObjectType, "Encountered pointer property of unknown type"))
					return false;

				if (is_embedded_pointer)
				{
					nap::Object* target = nullptr;
					if (!readEmbeddedObject(json_element, readObjects, unresolvedPointers, linkedFiles, target, errorState))
						return false;

					array.set_value(index, target);
				}
				else
				{
					// Determine the target of the pointer
					std::string target = std::string(json_element.GetString());

					// Add to list of unresolved pointers
					if (!target.empty())
						unresolvedPointers.push_back(UnresolvedPointer(rootObject, rttiPath, target));
				}

			}
			else
			{
				if (json_element.IsArray())
				{
					// Array-of-arrays; read array recursively
					RTTI::VariantArray sub_array = array.get_value_as_ref(index).create_array_view();
					if (!readArrayRecursively(rootObject, property, sub_array, json_element, rttiPath, readObjects, unresolvedPointers, linkedFiles, errorState))
						return false;
				}
				else if (json_element.IsObject())
				{
					// Array-of-compounds; read object recursively
					RTTI::Variant var_tmp = array.get_value_as_ref(index);
					RTTI::Variant wrapped_var = var_tmp.extract_wrapped_value();
					if (!readPropertiesRecursive(rootObject, wrapped_var, json_element, rttiPath, readObjects, unresolvedPointers, linkedFiles, errorState))
						return false;
					array.set_value(index, wrapped_var);
				}
				else
				{
					// Array of basic types; read basic type
					RTTI::Variant extracted_value = readBasicType(json_element);
					if (extracted_value.convert(array_type))
						array.set_value(index, extracted_value);
				}
			}			

			// Remove array element from rtti path again
			rttiPath.popBack();
		}

		return true;
	}

	nap::Object* readObjectRecursive(const rapidjson::Value::ConstMemberIterator& jsonObject, OwnedObjectList& readObjects, UnresolvedPointerList& unresolvedPointers,
		std::vector<FileLink>& linkedFiles, nap::ErrorState& errorState)
	{
		// Check whether the object is of a known type
		const char* typeName = jsonObject->name.GetString();
		RTTI::TypeInfo type_info = RTTI::TypeInfo::get_by_name(typeName);
		if (!errorState.check(type_info.is_valid(), "Unknown object type %s encountered.", typeName))
			return nullptr;

		// Check whether this is a type that can actually be instantiated
		if (!errorState.check(type_info.can_create_instance(), "Unable to instantiate object of type %s.", typeName))
			return nullptr;

		// We only support root-level objects that derive from nap::Object (compounds, etc can be of any type)
		if (!errorState.check(type_info.is_derived_from(RTTI_OF(nap::Object)), "Unable to instantiate object %s. Class is not derived from Object.", typeName))
			return nullptr;

		// Create new instance of the object
		Object* object = type_info.create<Object>();
		readObjects.push_back(std::unique_ptr<Object>(object));

		// Recursively read properties, nested compounds, etc
		RTTI::RTTIPath path;
		if (!readPropertiesRecursive(object, *object, jsonObject->value, path, readObjects, unresolvedPointers, linkedFiles, errorState))
			return nullptr;

		return object;
	}

	bool deserializeJSON(const std::string& json, RTTIDeserializeResult& result, nap::ErrorState& errorState)
	{
		// Try to parse the json file
		rapidjson::Document document;
		rapidjson::ParseResult parse_result = document.Parse(json.c_str());
		if (!parse_result)
		{
			// RapidJSON does not offer a way to find out the line that the error was on, so we do some custom work here to be able to print out
			// the offending line in the case of a parse error
			size_t error_line_start = json.rfind('\n', parse_result.Offset());
			size_t error_line_end = json.find('\n', parse_result.Offset());

			if (error_line_start == std::string::npos)
				error_line_start = 0;
			else
				error_line_start += 1;

			if (error_line_end == std::string::npos)
				error_line_end = json.size();
			else
				error_line_end -= 1;

			std::string error_line = json.substr(error_line_start, error_line_end - error_line_start);

			errorState.fail("Error parsing json: %s (line: %s)", rapidjson::GetParseError_En(parse_result.Code()), error_line.c_str());
			return false;
		}

		// Read objects
		for (auto object_pos = document.MemberBegin(); object_pos < document.MemberEnd(); ++object_pos)
		{
			if (!readObjectRecursive(object_pos, result.mReadObjects, result.mUnresolvedPointers, result.mFileLinks, errorState))
				return false;
		}

		return true;
	}


	bool readJSONFile(const std::string& path, RTTIDeserializeResult& result, nap::ErrorState& errorState)
	{
		// Open the file
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!errorState.check(in.good(), "Unable to open file %s", path.c_str()))
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

		if (!deserializeJSON(buffer, result, errorState))
			return false;

		return true;
	}
}
