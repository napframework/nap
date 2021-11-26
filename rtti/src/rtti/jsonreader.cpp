/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "jsonreader.h"
#include "factory.h"
#include "object.h"

// External Includes
#include <utility/errorstate.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>
#include <utility/fileutils.h>

namespace nap
{
	namespace rtti
	{

		static const std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID = "Generated")
		{
			std::string unique_id = baseID;

			int index = 1;
			while (objectIDs.find(unique_id) != objectIDs.end())
				unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

			objectIDs.insert(unique_id);

			return unique_id;
		}


		/**
		 * Helper function to give correct error message in case of pointer type mismatch.
		 */
		static bool checkPointerProperty(const ReadState& readState, bool isRawPointer, const std::string& rootObjectID, utility::ErrorState& errorState)
		{
			if (readState.mPointerPropertyMode == EPointerPropertyMode::OnlyRawPointers && !isRawPointer)
			{
				errorState.fail("Encountered a non-raw pointer property {%s}:%s while only raw pointers are supported.", rootObjectID.c_str(), readState.mCurrentRTTIPath.toString().c_str());
				return false;
			}

			if (readState.mPointerPropertyMode == EPointerPropertyMode::NoRawPointers && isRawPointer)
			{
				errorState.fail("Encountered a raw pointer property {%s}:%s while only pointers like ObjectPtr are supported.", rootObjectID.c_str(), readState.mCurrentRTTIPath.toString().c_str());
				return false;
			}

			return true;
		}


		static bool readArrayRecursively(rtti::Object* rootObject, const rtti::Property& property, rtti::VariantArray& array, const rapidjson::Value& jsonArray, ReadState& readState, utility::ErrorState& errorState);

		static rtti::Object* readObjectRecursive(const rapidjson::Value& jsonObject, bool isEmbeddedObject, ReadState& readState, utility::ErrorState& errorState);

		/**
		 * Helper function to read a basic JSON type to a C++ type
		 */
		static rtti::Variant readBasicType(const rapidjson::Value& jsonValue)
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
				default:
					break;
			}

			// Unknown type
			assert(false);
			return rtti::Variant();
		}


		static bool readEmbeddedObject(const rapidjson::Value& jsonValue, ReadState& readState, rtti::Object*& resultObject, utility::ErrorState& errorState)
		{
			resultObject = nullptr;

			if (jsonValue.GetType() == rapidjson::kStringType)
			{
				if (!errorState.check(std::string(jsonValue.GetString()).empty(), "Encountered embedded pointer that points to a non-embedded object"))
					return false;
			}
			else
			{
				// Because we're entering a new embedded object, we need to reset the 'current' RTTIPath to restart with the nested object
				Path old_rtti_path = readState.mCurrentRTTIPath;
				readState.mCurrentRTTIPath = Path();

				// Deserialize the nested object
				resultObject = readObjectRecursive(jsonValue, true, readState, errorState);
				if (resultObject == nullptr)
					return false;

				// After reading the embedded object, we need to put the 'current' RTTIPath back to what it was
				readState.mCurrentRTTIPath = old_rtti_path;
			}

			return true;
		}

		/**
		 * Helper function to recursively read an object (can be a rtti::RTTIObject, nested compound or any other type) from JSON
		 */
		static bool readPropertiesRecursive(rtti::Object* rootObject, rtti::Instance compound, const rapidjson::Value& jsonCompound, bool isEmbeddedObject, ReadState& readState, utility::ErrorState& errorState)
		{
			// Determine the object type. Note that we want to *most derived type* of the object.
			rtti::TypeInfo object_type = compound.get_derived_type();

			// Go through all properties of the object
			for (const rtti::Property& property : object_type.get_properties())
			{
				// Push attribute on path
				readState.mCurrentRTTIPath.pushAttribute(property.get_name().data());

				// Determine meta-data for the property
				bool is_required = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Required) && readState.mPropertyValidationMode == EPropertyValidationMode::DisallowMissingProperties;
				bool is_file_link = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::FileLink);
				bool is_object_id = Object::isIDProperty(compound, property);

				// Check whether the property is present in the JSON. If it's not, but the property is required, throw an error
				rapidjson::Value::ConstMemberIterator json_property = jsonCompound.FindMember(property.get_name().data());
				if (json_property == jsonCompound.MemberEnd())
				{
					// If this is the ObjectID property and we're allowing missing ids, generate one
					if (is_object_id && isEmbeddedObject)
					{
						rootObject->mID = generateUniqueID(readState.mObjectIDs, rootObject->get_type().get_name().data());
					}
					else
					{
						// Otherwise make sure the property is not required
						if (!errorState.check(!is_required || (is_object_id && isEmbeddedObject), "Required property %s not found in object of type %s", property.get_name().data(), object_type.get_name().data()))
							return false;
					}

					readState.mCurrentRTTIPath.popBack();
					continue;
				}

				const rtti::TypeInfo value_type = property.get_type();
				const rtti::TypeInfo wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;

				const rapidjson::Value& json_value = json_property->value;

				// If this is a file link, make sure it's of the expected type (string)
				if (!errorState.check((is_file_link && wrapped_type.get_raw_type().is_derived_from<std::string>()) || !is_file_link, "Encountered a non-string file link. This is not supported"))
					return false;

				// If the property is of pointer type, we can't set the property here but need to resolve it later
				if (wrapped_type.is_pointer())
				{
					bool is_raw_pointer = wrapped_type == value_type;
					if (!checkPointerProperty(readState, is_raw_pointer, rootObject->mID, errorState))
						return false;

					// Pointer types must point to objects derived from rtti::RTTIObject
					if (!errorState.check(wrapped_type.get_raw_type().is_derived_from<rtti::Object>(), "Encountered pointer to non-Object. This is not supported"))
						return false;

					bool is_embedded_pointer = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Embedded);

					// Check if type in json is the expected type
					if (is_embedded_pointer)
					{
						bool valid_type = json_value.GetType() == rapidjson::kStringType || json_value.GetType() == rapidjson::kObjectType;
						if (!errorState.check(valid_type, "Encountered invalid embedded pointer property value for property {%s}:%s (must be a string or an object)", rootObject->mID.c_str(), readState.mCurrentRTTIPath.toString().c_str()))
							return false;
					}
					else
					{
						bool valid_type = json_value.GetType() == rapidjson::kStringType;
						if (!errorState.check(valid_type, "Encountered invalid pointer property value for property {%s}:%s (must be a string for non-embedded pointer types)", rootObject->mID.c_str(), readState.mCurrentRTTIPath.toString().c_str()))
							return false;
					}

					// Determine the target of the pointer
					std::string target_id;
					if (is_embedded_pointer)
					{
						// Read embedded object
						rtti::Object* target = nullptr;
						if (!readEmbeddedObject(json_value, readState, target, errorState))
							return false;

						if (target != nullptr)
						{
							assert(!target->mID.empty());
							target_id = target->mID;
						}							
					}
					else
					{
						target_id = std::string(json_value.GetString());
					}

					// If the target is empty (i.e. null pointer), but the property is required, throw an error
					if (!errorState.check((is_required && !target_id.empty()) || (!is_required), "Required property %s not found in object of type %s", property.get_name().data(), object_type.get_name().data()))
						return false;

					// Add to list of unresolved pointers
					if (!target_id.empty())
						readState.mResult.mUnresolvedPointers.push_back(UnresolvedPointer(rootObject, readState.mCurrentRTTIPath, target_id));
				}
                else if (wrapped_type.is_enumeration())
                {
                    rtti::Variant extracted_enum_string = readBasicType(json_value);
                    rtti::Enum enumeration = wrapped_type.get_enumeration();
                    rtti::Variant enum_value = enumeration.name_to_value(extracted_enum_string.get_value<std::string>());

                    if (!errorState.check(enum_value.is_valid(),
                        "Failed to extract has type: %s, object: %s", readState.mCurrentRTTIPath.toString().c_str(),
                        rootObject->mID.c_str()))
                    {
                        if (readState.mCurrentRTTIPath.toString() == "Type")
                            errorState.fail("Type is a reserved keyword");
                        return false;
                    }

                    property.set_value(compound, enum_value);
                }
				else
				{
					// Regular property; read the value and set the property
					switch (json_value.GetType())
					{
						case rapidjson::kArrayType:
						{
							// If this is an array, read its elements recursively again (in case of nested compounds)
							rtti::Variant value;
							if (wrapped_type.is_array())
							{
								// Get instance of the current value (this is a copy) and create an array view on it so we can fill it
								value = property.get_value(compound);
								rtti::VariantArray array_view = value.create_array_view();

								// Now read the array recursively into array view
								if (!readArrayRecursively(rootObject, property, array_view, json_value, readState, errorState))
									return false;
							}
							else if (wrapped_type.is_associative_container())
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
							rtti::Variant var = property.get_value(compound);
							if (!readPropertiesRecursive(rootObject, var, json_value, false, readState, errorState))
								return false;

							// Copy read object back into the target object
							property.set_value(compound, var);
							break;
						}
						default:
						{
							// Basic JSON type, read value and copy to target
							rtti::Variant extracted_value = readBasicType(json_value);
							if (!errorState.check(extracted_value.convert(wrapped_type),
								"Failed to extract primitive type: %s, object: %s", readState.mCurrentRTTIPath.toString().c_str(),
								rootObject->mID.c_str()))
							{
								if (readState.mCurrentRTTIPath.toString() == "Type")
									errorState.fail("Type is a reserved keyword");
								return false;
							}

							property.set_value(compound, extracted_value);
						}
					}
				}

				// If this property is a file link, add it to the list of file links
				if (is_file_link)
				{
					FileLink file_link;
					file_link.mTargetFile = property.get_value(compound).get_value<std::string>();
					readState.mResult.mFileLinks.push_back(file_link);
				}

				// If this property is the object ID, do some validation
				if (is_object_id)
				{
					// Make sure the ID is not empty
					if (!errorState.check(!rootObject->mID.empty(), "Object of type %s doesn't have an ID", object_type.get_name().data()))
						return false;

					// Make sure it's not a duplicate ID
					if (!errorState.check(readState.mObjectIDs.find(rootObject->mID) == readState.mObjectIDs.end(),
							"Encountered object of type %s with duplicate ID %s", object_type.get_name().data(), rootObject->mID.c_str()))
					{
						return false;
					}

					// Add to set of IDs
					readState.mObjectIDs.insert(rootObject->mID);
				}

				readState.mCurrentRTTIPath.popBack();
			}

			return true;
		}


		/**
		 * Helper function to recursively read an array (can be an array of basic types, nested compound, or any other type) from JSON
		 */
		static bool readArrayRecursively(rtti::Object* rootObject, const rtti::Property& property, rtti::VariantArray& array, const rapidjson::Value& jsonArray, ReadState& readState, utility::ErrorState& errorState)
		{
			// Pre-size the array to avoid too many dynamic allocs
			array.set_size(jsonArray.Size());

			// Determine the rank of the array (i.e. how many dimensions it has)
			const rtti::TypeInfo array_type = array.get_rank_type(array.get_rank());
			const rtti::TypeInfo wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;

			// Read values from JSON array
			for (std::size_t index = 0; index < jsonArray.Size(); ++index)
			{
				// Add array element to rtti path
				readState.mCurrentRTTIPath.pushArrayElement(index);

				const rapidjson::Value& json_element = jsonArray[index];

				if (wrapped_type.is_pointer())
				{
					bool is_raw_pointer = wrapped_type == array_type;
					if (!checkPointerProperty(readState, is_raw_pointer, rootObject->mID, errorState))
						return false;

					// Pointer types must point to objects derived from rtti::RTTIObject
					if (!errorState.check(wrapped_type.get_raw_type().is_derived_from<rtti::Object>(), "Encountered pointer to non-Object. This is not supported"))
						return false;

					bool is_embedded_pointer = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Embedded);

					// Check if type in json is the expected type
					if (is_embedded_pointer)
					{
						bool valid_type = json_element.GetType() == rapidjson::kStringType || json_element.GetType() == rapidjson::kObjectType;
						if (!errorState.check(valid_type, "Encountered invalid embedded pointer property value for property {%s}:%s (must be a string or an object)", rootObject->mID.c_str(), readState.mCurrentRTTIPath.toString().c_str()))
							return false;
					}
					else
					{
						bool valid_type = json_element.GetType() == rapidjson::kStringType;
						if (!errorState.check(valid_type, "Encountered invalid pointer property value for property {%s}:%s (must be a string for non-embedded pointer types)", rootObject->mID.c_str(), readState.mCurrentRTTIPath.toString().c_str()))
							return false;
					}

					std::string target_id;
					if (is_embedded_pointer)
					{
						rtti::Object* target = nullptr;
						if (!readEmbeddedObject(json_element, readState, target, errorState))
							return false;

						if (target != nullptr)
						{
							assert(!target->mID.empty());
							target_id = target->mID;
						}
					}
					else
					{
						// Determine the target of the pointer
						target_id = std::string(json_element.GetString());
					}

					// Add to list of unresolved pointers
					if (!target_id.empty())
						readState.mResult.mUnresolvedPointers.push_back(UnresolvedPointer(rootObject, readState.mCurrentRTTIPath, target_id));
				}
				else
				{
					if (json_element.IsArray())
					{
						// Array-of-arrays; read array recursively
						rtti::VariantArray sub_array = array.get_value_as_ref(index).create_array_view();
						if (!readArrayRecursively(rootObject, property, sub_array, json_element, readState, errorState))
							return false;
					}
					else if (json_element.IsObject())
					{
						// Array-of-compounds; read object recursively
						rtti::Variant var_tmp = array.get_value_as_ref(index);
						rtti::Variant wrapped_var = var_tmp.extract_wrapped_value();
						if (!readPropertiesRecursive(rootObject, wrapped_var, json_element, false, readState, errorState))
							return false;
						array.set_value(index, wrapped_var);
					}
					else
					{
						// Array of basic types; read basic type
						rtti::Variant extracted_value = readBasicType(json_element);
						if (!errorState.check(extracted_value.convert(wrapped_type),
							"Failed to extract primitive type: %s, object: %s", readState.mCurrentRTTIPath.toString().c_str(),
							rootObject->mID.c_str()))
						{
							if (readState.mCurrentRTTIPath.toString() == "Type")
								errorState.fail("Type is a reserved keyword");
							return false;
						}
							
						
						array.set_value(index, extracted_value);
					}
				}

				// Remove array element from rtti path again
				readState.mCurrentRTTIPath.popBack();
			}

			return true;
		}

		static rtti::Object* readObjectRecursive(const rapidjson::Value& jsonObject, bool isEmbeddedObject,
										  ReadState& readState, utility::ErrorState& errorState)
		{
			// Check whether the object is of a known type
			rapidjson::Value::ConstMemberIterator type = jsonObject.FindMember("Type");
			if (!errorState.check(type != jsonObject.MemberEnd(), "Unable to find required 'Type' property for object"))
				return nullptr;

			const char* typeName = type->value.GetString();
			rtti::TypeInfo type_info = rtti::TypeInfo::get_by_name(typeName);
			if (!errorState.check(type_info.is_valid(), "Unknown object type %s encountered.", typeName))
				return nullptr;

			// Check whether this is a type that can actually be instantiated
			if (!errorState.check(readState.mFactory.canCreate(type_info), "Unable to instantiate object of type %s.", typeName))
				return nullptr;

			// We only support root-level objects that derive from rtti::RTTIObject (compounds, etc can be of any type)
			if (!errorState.check(type_info.is_derived_from(RTTI_OF(rtti::Object)),
								  "Unable to instantiate object %s. Class is not derived from %s.",
								  typeName, RTTI_OF(rtti::Object).get_name().data()))
				return nullptr;

			// Create new instance of the object
			Object* object = readState.mFactory.create(type_info);
			if (!errorState.check(object != nullptr, "Failed to instantiate object of type %s.", typeName))
				return nullptr;

			readState.mResult.mReadObjects.push_back(std::unique_ptr<Object>(object));

			// Recursively read properties, nested compounds, etc
			rtti::Path path;
			if (!readPropertiesRecursive(object, *object, jsonObject, isEmbeddedObject, readState, errorState))
				return nullptr;

			return object;
		}


		bool deserializeJSON(const std::string& json, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState)
		{
			rapidjson::Document document;
			if (!JSONDocumentFromString(json, document, errorState))
				return false;

			// Read objects
			if (!errorState.check(document.HasMember("Objects"), "Unable to find required 'Objects' field"))
				return false;

			return deserializeObjects(document["Objects"], propertyValidationMode, pointerPropertyMode, factory, result, errorState);
		}

		bool deserializeObjects(const rapidjson::Value& objects,
									  EPropertyValidationMode propertyValidationMode,
									  EPointerPropertyMode pointerPropertyMode,
									  Factory& factory,
									  DeserializeResult& result,
									  utility::ErrorState& errorState)
		{
			ReadState readState(propertyValidationMode, pointerPropertyMode, factory, result);

			if (!errorState.check(objects.IsArray(), "Objects field must be an array"))
				return false;

			for (std::size_t index = 0; index < objects.Size(); ++index)
			{
				const rapidjson::Value& json_element = objects[index];
				if (!readObjectRecursive(json_element, false, readState, errorState))
					return false;
			}

			return true;
		}

		bool deserializeJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, EPointerPropertyMode pointerPropertyMode, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState)
		{
			std::string buffer;
			if (!utility::readFileToString(path, buffer, errorState))
				return false;

			return deserializeJSON(buffer, propertyValidationMode, pointerPropertyMode, factory, result, errorState);
		}


		std::unique_ptr<rtti::Object> getObjectFromJSONFile(const std::string& path, EPropertyValidationMode propertyValidationMode, Factory& factory, utility::ErrorState& errorState)
		{
			std::string buffer;
			if (!utility::readFileToString(path, buffer, errorState))
				return {nullptr};

			rapidjson::Document document;
			if (!JSONDocumentFromString(buffer, document, errorState))
				return {nullptr};

			if (!errorState.check(document.IsObject(), "Document must be an Object"))
				return {nullptr};

			DeserializeResult result;
			ReadState readState(propertyValidationMode, rtti::EPointerPropertyMode::OnlyRawPointers, factory, result);
			if (!readObjectRecursive(document, false, readState, errorState))
				return {nullptr};

			return std::move(result.mReadObjects[0]);
		}


		bool JSONDocumentFromString(const std::string& json, rapidjson::Document& document, nap::utility::ErrorState& errorState)
		{
			rapidjson::ParseResult parse_result = document.Parse(json.c_str());
			if (!parse_result)
			{
				errorState.fail("Error parsing json: %s (line: %d)",
					rapidjson::GetParseError_En(parse_result.Code()),
					utility::getLine(json, parse_result.Offset()));
				return false;
			}
			return true;
		}

	}
}
