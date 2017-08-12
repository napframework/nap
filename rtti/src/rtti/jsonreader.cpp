#include "jsonreader.h"
#include "factory.h"
#include "utility/errorstate.h"
#include "rtti/rttiobject.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	namespace rtti
	{
		struct ReadState
		{
			ReadState(Factory& factory, RTTIDeserializeResult& result) :
				mFactory(factory),
				mResult(result)
			{
			}

			RTTIPath							mCurrentRTTIPath;
			Factory&							mFactory;
			RTTIDeserializeResult&				mResult;
			std::unordered_set<std::string>		mObjectIDs;
		};

		static const std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID = "Generated")
		{
			std::string unique_id = baseID;

			int index = 1;
			while (objectIDs.find(unique_id) != objectIDs.end())
				unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

			objectIDs.insert(unique_id);

			return unique_id;
		}

		static bool readArrayRecursively(rtti::RTTIObject* rootObject, const rtti::Property& property, rtti::VariantArray& array, const rapidjson::Value& jsonArray, ReadState& readState, utility::ErrorState& errorState);

		static rtti::RTTIObject* readObjectRecursive(const rapidjson::Value::ConstMemberIterator& jsonObject, bool isEmbeddedObject, ReadState& readState, utility::ErrorState& errorState);

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
			return rtti::Variant();
		}


		static bool readEmbeddedObject(const rapidjson::Value& jsonValue, ReadState& readState, rtti::RTTIObject*& resultObject, utility::ErrorState& errorState)
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

				// Because we're entering a new embedded object, we need to reset the 'current' RTTIPath to restart with the nested object
				RTTIPath old_rtti_path = readState.mCurrentRTTIPath;
				readState.mCurrentRTTIPath = RTTIPath();

				// Deserialize the nested object
				resultObject = readObjectRecursive(jsonValue.MemberBegin(), true, readState, errorState);
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
		static bool readPropertiesRecursive(rtti::RTTIObject* rootObject, rtti::Instance compound, const rapidjson::Value& jsonCompound, bool isEmbeddedObject, ReadState& readState, utility::ErrorState& errorState)
		{
			// Determine the object type. Note that we want to *most derived type* of the object.
			rtti::TypeInfo object_type = compound.get_derived_type();

			// Go through all properties of the object
			for (const rtti::Property& property : object_type.get_properties())
			{
				// Push attribute on path
				readState.mCurrentRTTIPath.pushAttribute(property.get_name().data());

				// Determine meta-data for the property
				bool is_required = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Required);
				bool is_file_link = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::FileLink);
				bool is_object_id = RTTIObject::isIDProperty(compound, property);

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
					// Pointer types must point to objects derived from rtti::RTTIObject
					if (!errorState.check(wrapped_type.get_raw_type().is_derived_from<rtti::RTTIObject>(), "Encountered pointer to non-Object. This is not supported"))
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
						rtti::RTTIObject* target = nullptr;
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
							if (!errorState.check(extracted_value.convert(wrapped_type), "Failed to extract primitive type"))
								return false;

							property.set_value(compound, extracted_value);
						}
					}
				}

				// If this property is a file link, add it to the list of file links
				if (is_file_link)
				{
					FileLink file_link;
					file_link.mSourceObjectID = compound.try_convert<RTTIObject>()->mID;
					file_link.mTargetFile = property.get_value(compound).get_value<std::string>();;
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
		static bool readArrayRecursively(rtti::RTTIObject* rootObject, const rtti::Property& property, rtti::VariantArray& array, const rapidjson::Value& jsonArray, ReadState& readState, utility::ErrorState& errorState)
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
					// Pointer types must point to objects derived from rtti::RTTIObject
					if (!errorState.check(wrapped_type.get_raw_type().is_derived_from<rtti::RTTIObject>(), "Encountered pointer to non-Object. This is not supported"))
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
						rtti::RTTIObject* target = nullptr;
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
						if (!errorState.check(extracted_value.convert(wrapped_type), "Failed to extract primitive type"))
							return false;
						
						array.set_value(index, extracted_value);
					}
				}

				// Remove array element from rtti path again
				readState.mCurrentRTTIPath.popBack();
			}

			return true;
		}

		rtti::RTTIObject* readObjectRecursive(const rapidjson::Value::ConstMemberIterator& jsonObject, bool isEmbeddedObject, ReadState& readState, utility::ErrorState& errorState)
		{
			// Check whether the object is of a known type
			const char* typeName = jsonObject->name.GetString();
			rtti::TypeInfo type_info = rtti::TypeInfo::get_by_name(typeName);
			if (!errorState.check(type_info.is_valid(), "Unknown object type %s encountered.", typeName))
				return nullptr;

			// Check whether this is a type that can actually be instantiated
			if (!errorState.check(type_info.can_create_instance(), "Unable to instantiate object of type %s.", typeName))
				return nullptr;

			// We only support root-level objects that derive from rtti::RTTIObject (compounds, etc can be of any type)
			if (!errorState.check(type_info.is_derived_from(RTTI_OF(rtti::RTTIObject)), "Unable to instantiate object %s. Class is not derived from RTTIObject.", typeName))
				return nullptr;

			// Create new instance of the object
			RTTIObject* object = readState.mFactory.create(type_info);
			readState.mResult.mReadObjects.push_back(std::unique_ptr<RTTIObject>(object));

			// Recursively read properties, nested compounds, etc
			rtti::RTTIPath path;
			if (!readPropertiesRecursive(object, *object, jsonObject->value, isEmbeddedObject, readState, errorState))
				return nullptr;

			return object;
		}


		int getLine(const std::string& json, size_t offset)
		{
			int line = 1;
			int line_offset = 0;
			while (true)
			{
				line_offset = json.find('\n', line_offset);
				if (line_offset == std::string::npos || line_offset > offset)
					break;
				++line;
				line_offset += 1;
			}
			return line;
		}


		bool deserializeJSON(const std::string& json, Factory& factory, RTTIDeserializeResult& result, utility::ErrorState& errorState)
		{
			// Try to parse the json file
			rapidjson::Document document;
			rapidjson::ParseResult parse_result = document.Parse(json.c_str());
			if (!parse_result)
			{
				errorState.fail("Error parsing json: %s (line: %d)", rapidjson::GetParseError_En(parse_result.Code()), getLine(json, parse_result.Offset()));
				return false;
			}

			// Read objects
			ReadState readState(factory, result);
			for (auto object_pos = document.MemberBegin(); object_pos < document.MemberEnd(); ++object_pos)
			{
				if (!readObjectRecursive(object_pos, false, readState, errorState))
					return false;
			}

			return true;
		}


		bool readJSONFile(const std::string& path, Factory& factory, RTTIDeserializeResult& result, utility::ErrorState& errorState)
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

			if (!deserializeJSON(buffer, factory, result, errorState))
				return false;

			return true;
		}
	}
}
