#include "resourcemanager.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>

RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{
	class JSONReader
	{
	public:
		struct UnresolvedPointer
		{
			UnresolvedPointer(RTTI::Instance& object, const RTTI::Property& property, const std::string& targetID) :
				mObject(object),
				mProperty(property),
				mTargetID(targetID)
			{
			}

			RTTI::Instance mObject;
			RTTI::Property mProperty;
			std::string mTargetID;
		};

		using ObjectList = std::vector<nap::Object*>;
		using UnresolvedPointerList = std::vector<UnresolvedPointer>;

		bool Read(const std::string& filename, ObjectList& readObjects, nap::InitResult& initResult)
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
				if (end == std::string::npos)
					end = buffer.size();

				std::string error_line = buffer.substr(start, end - start);

				initResult.mErrorString = nap::stringFormat("Error parsing %s: %s (line: %s)", filename.c_str(), rapidjson::GetParseError_En(parse_result.Code()), error_line.c_str());
				return false;
			}

			std::vector<RTTI::Variant> read_objects;

			UnresolvedPointerList unresolved_pointers;
			for (auto& object_pos = document.MemberBegin(); object_pos < document.MemberEnd(); ++object_pos)
			{
				const char* typeName = object_pos->name.GetString();
				RTTI::TypeInfo type_info = RTTI::TypeInfo::get_by_name(typeName);
				if (!initResult.check(type_info.is_valid(), "Unknown object type %s encountered.", typeName))
					return false;

				if (!initResult.check(type_info.can_create_instance(), "Unable to instantiate object of type %s.", typeName))
					return false;

				if (!initResult.check(type_info.is_derived_from(RTTI_OF(nap::Object)), "Unable to instantiate object %s. Class is not derived from Object.", typeName))
					return false;

				RTTI::Variant object = type_info.create();
				read_objects.push_back(object);

				if (!ReadObjectRecursive(object, object_pos->value, unresolved_pointers, initResult))
					return false;
			}

			using ObjectMap = std::unordered_map<std::string, RTTI::Variant>;
			ObjectMap objects_by_id;
			for (RTTI::Variant object : read_objects)
			{
				Object* actual_object = object.get_value<Object*>();
				if (!initResult.check(!actual_object->mID.empty(), "Encountered an object without an ID"))
					return false;

				if (!initResult.check(objects_by_id.find(actual_object->mID) == objects_by_id.end(), "Encountered an object with a duplicate ID %s", actual_object->mID))
					return false;

				objects_by_id.insert({ actual_object->mID, object });
				readObjects.push_back(actual_object);
			}

			for (const UnresolvedPointer& unresolved_pointer : unresolved_pointers)
			{
				ObjectMap::iterator target = objects_by_id.find(unresolved_pointer.mTargetID);
				if (!initResult.check(target != objects_by_id.end(), "Unable to resolve link to object %s from attribute %s", unresolved_pointer.mTargetID.c_str(), unresolved_pointer.mProperty.get_name().data()))
					return false;

				bool succeeded = unresolved_pointer.mProperty.set_value(unresolved_pointer.mObject, target->second);
				if (!initResult.check(succeeded, "Failed to resolve pointer"))
					return false;
			}
			 
			return true;
		}

		bool ReadObjectRecursive(RTTI::Instance object, const rapidjson::Value& jsonObject, UnresolvedPointerList& unresolvedPointers, InitResult& initResult)
		{
			for (const RTTI::Property& property : object.get_derived_type().get_properties())
			{
				rapidjson::Value::ConstMemberIterator json_property = jsonObject.FindMember(property.get_name().data());
				if (json_property == jsonObject.MemberEnd())
					continue;
				
				const RTTI::TypeInfo value_type = property.get_type();
				const rapidjson::Value& json_value = json_property->value;

				if (value_type.is_pointer())
				{
					if (!initResult.check(value_type.get_raw_type().is_derived_from<nap::Object>(), "Encountered pointer to non-Object. This is not supported"))
						return false;
					
					if (!initResult.check(json_value.GetType() == rapidjson::kStringType, "Encountered pointer property of unknown type"))
						return false;

					std::string target = std::string(json_value.GetString());
					unresolvedPointers.push_back(UnresolvedPointer(object, property, target));
				}
				else
				{
					switch(json_value.GetType())
					{
						case rapidjson::kArrayType:
						{
							RTTI::Variant value;
							if (value_type.is_array())
							{
								value = property.get_value(object);
								auto array_view = value.create_array_view();
								if (!ReadArrayRecursively(array_view, json_value, unresolvedPointers, initResult))
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
							if (!ReadObjectRecursive(var, json_value, unresolvedPointers, initResult))
								return false;
							property.set_value(object, var);
							break;
						}
						default:
						{
							RTTI::Variant extracted_value = ReadBasicType(json_value);
							if (extracted_value.convert(value_type))
								property.set_value(object, extracted_value);
						}
					}
				}				
			}

			return true;
		}

		RTTI::Variant ReadBasicType(const rapidjson::Value& json_value)
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

		bool ReadArrayRecursively(RTTI::VariantArray& array, const rapidjson::Value& jsonArray, UnresolvedPointerList& unresolvedPointers, InitResult& initResult)
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
					if (!ReadArrayRecursively(sub_array, json_element, unresolvedPointers, initResult))
						return false;
				}
				else if (json_element.IsObject())
				{
					RTTI::Variant var_tmp = array.get_value_as_ref(i);
					RTTI::Variant wrapped_var = var_tmp.extract_wrapped_value();
					if (!ReadObjectRecursive(wrapped_var, json_element, unresolvedPointers, initResult))
						return false;
					array.set_value(i, wrapped_var);
				}
				else
				{
					RTTI::Variant extracted_value = ReadBasicType(json_element);
					if (extracted_value.convert(array_type))
						array.set_value(i, extracted_value);
				}
			}

			return true;					
		}
	};

	bool ResourceManagerService::loadFile(const std::string& filename, nap::InitResult& initResult)
	{
		std::vector<Object*> objects;
		JSONReader reader;
		if (!reader.Read(filename, objects, initResult))
			return false;

		for (Object* object : objects)
		{
			nap::Resource* resource = rtti_cast<nap::Resource>(object);
			if (resource == nullptr)
				continue;

			if (!resource->init(initResult))
				return false;
		}

		for (Object* object : objects)
		{
			nap::Resource* resource = rtti_cast<nap::Resource>(object);
			if (resource == nullptr)
				continue;

			addResource(resource->mID, resource);
		}

		return true;
	}

	Resource* ResourceManagerService::findResource(const std::string& id)
	{
		const auto& it = mResources.find(id);
		
		if (it != mResources.end())
			return it->second.get();
		
		return nullptr;
	}

	void ResourceManagerService::addResource(const std::string& id, Resource* resource)
	{
		assert(mResources.find(id) == mResources.end());
		mResources.emplace(id, std::move(std::unique_ptr<Resource>(resource)));
	}

	Resource* ResourceManagerService::createResource(const RTTI::TypeInfo& type)
	{
		if (!type.is_derived_from(RTTI_OF(Resource)))
		{
			nap::Logger::warn("unable to create resource of type: %s", type.get_name().data());
			return nullptr;
		}

		if (!type.can_create_instance())
		{
			nap::Logger::warn("can't create resource instance of type: %s", type.get_name().data());
			return nullptr;
		}

		// Create instance of resource
		Resource* resource = type.create<Resource>();

		// Construct path
		std::string type_name = type.get_name().data();
		std::string reso_path = stringFormat("resource::%s", type_name.c_str());
		std::string reso_unique_path = reso_path;
		int idx = 0;
		while (mResources.find(reso_unique_path) != mResources.end())
		{
			++idx;
			reso_unique_path = stringFormat("%s_%d", reso_path.c_str(), idx);
		}
		
		resource->mID = reso_unique_path;
		addResource(reso_unique_path, resource);
		
		return resource;
	}
}