#include "resourcemanager.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#undef min
#undef max

RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{
	static void copyObject(const Object& srcObject, Object& dstObject)
	{
		// TODO: assert types compatible
		RTTI::TypeInfo type = srcObject.get_type();

		for (const RTTI::Property& property : type.get_properties())
		{
			RTTI::Variant new_value = property.get_value(srcObject);
			property.set_value(dstObject, new_value);
		}
	}

	template<typename T>
	static T* cloneObject(T& object)
	{
		RTTI::TypeInfo type = object.get_type();
		T* copy = type.create<T>();
		copyObject(object, *copy);

		return copy;
	}

	struct DirectoryWatcher
	{
	public:
		DirectoryWatcher()
		{
			char current_directory[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, current_directory);

			mDirectoryToMonitor = CreateFileA(current_directory, FILE_LIST_DIRECTORY, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
			assert(mDirectoryToMonitor != INVALID_HANDLE_VALUE);

			mOverlappedEvent = CreateEvent(NULL, true, false, NULL);
			
			std::memset(&mNotifications, 0, sizeof(mNotifications));
			std::memset(&mOverlapped, 0, sizeof(mOverlapped));
			mOverlapped.hEvent = mOverlappedEvent;

			DWORD result = ReadDirectoryChangesW(mDirectoryToMonitor, (LPVOID)&mNotifications, sizeof(mNotifications), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &mOverlapped, NULL);
			assert(result != 0 || GetLastError() == ERROR_IO_PENDING);
		}

		~DirectoryWatcher()
		{
			CloseHandle(mDirectoryToMonitor);
			CloseHandle(mOverlappedEvent);
		}

		bool update(std::string& modifiedFile)
		{
			bool did_update = false;

			DWORD result = WaitForSingleObject(mOverlappedEvent, 0);
			if (result == WAIT_OBJECT_0)
			{
				DWORD dwBytesReturned = 0;
				result = GetOverlappedResult(mDirectoryToMonitor, &mOverlapped, &dwBytesReturned, FALSE);
				if (result != 0)
				{
					modifiedFile.resize(mNotifications[0].FileNameLength / 2);
					
					wcstombs(&modifiedFile[0], mNotifications->FileName, mNotifications[0].FileNameLength / 2);
					did_update = true;

					ResetEvent(mOverlappedEvent);
					std::memset(&mNotifications, 0, sizeof(mNotifications));

					std::memset(&mOverlapped, 0, sizeof(mOverlapped));
					mOverlapped.hEvent = mOverlappedEvent;

					result = ReadDirectoryChangesW(mDirectoryToMonitor, (LPVOID)&mNotifications, sizeof(mNotifications), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &mOverlapped, NULL);
					assert(result != 0 || GetLastError() == ERROR_IO_PENDING);
				}
			}

			return did_update;
		}

		HANDLE mDirectoryToMonitor;
		HANDLE mOverlappedEvent;
		OVERLAPPED mOverlapped;	
		FILE_NOTIFY_INFORMATION mNotifications[1024];
	};

	class JSONReader
	{
	public:
		bool Read(const std::string& filename, ObjectList& readObjects, UnresolvedPointerList& unresolvedPointers, nap::InitResult& initResult)
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

				if (!ReadObjectRecursive(*object, object_pos->value, unresolvedPointers, initResult))
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

		bool ReadObjectRecursive(RTTI::Instance object, const rapidjson::Value& jsonObject, UnresolvedPointerList& unresolvedPointers, InitResult& initResult)
		{
			for (const RTTI::Property& property : object.get_derived_type().get_properties())
			{
				bool is_required = property.get_metadata(RTTI::EPropertyMetaData::Required).is_valid();

				rapidjson::Value::ConstMemberIterator json_property = jsonObject.FindMember(property.get_name().data());
				if (json_property == jsonObject.MemberEnd())
				{
					if (!initResult.check(!is_required, "Required property %s not found in object of type %s", property.get_name().data(), object.get_derived_type().get_name().data()))
						return false;
					continue;
				}

				const RTTI::TypeInfo value_type = property.get_type();
				const rapidjson::Value& json_value = json_property->value;

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

	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(new DirectoryWatcher())
	{
	}

	void ResourceManagerService::splitObjects(const ObjectList& sourceObjectList, ObjectList& targetObjectList, ExistingObjectMap& existingObjectMap, ObjectList& newObjectList)
	{
		for (Object* source_object : sourceObjectList)
		{
			Resource* resource = rtti_cast<Resource>(source_object);
			if (resource == nullptr)
				continue;

			std::string id = resource->mID;

			Resource* existing_resource = findResource(id);
			if (existing_resource == nullptr)
			{
				newObjectList.push_back(source_object);
				targetObjectList.push_back(source_object);
			}
			else
			{
				existingObjectMap.insert({ source_object, existing_resource });
				targetObjectList.push_back(existing_resource);
			}
		}
	}

	static int findUnresolvedPointer(UnresolvedPointerList& unresolvedPointers, Object* object, const RTTI::Property& property)
	{
		for (int index = 0; index < unresolvedPointers.size(); ++index)
		{
			UnresolvedPointer& unresolved_pointer = unresolvedPointers[index];
			if (unresolved_pointer.mObject == object &&
				unresolved_pointer.mProperty == property)
			{
				return index;
			}
		}

		return -1;
	}

	bool ResourceManagerService::updateExistingObjects(const ExistingObjectMap& existingObjectMap, UnresolvedPointerList& unresolvedPointers, InitResult& initResult)
	{
		for (auto kvp : existingObjectMap)
		{
			Resource* resource = rtti_cast<Resource>(kvp.first);
			if (resource == nullptr)
				continue;

			Resource* existing_resource = rtti_cast<Resource>(kvp.second);
			assert(existing_resource != nullptr);

			if (!initResult.check(existing_resource->get_type() == resource->get_type(), "Unable to update object, different types"))		// todo: actually support this properly
				return false;

			for (const RTTI::Property& property : resource->get_type().get_properties())
			{
				if (property.get_type().is_pointer())
				{
					int unresolved_pointer_index = findUnresolvedPointer(unresolvedPointers, resource, property);
					assert(unresolved_pointer_index != -1);

					unresolvedPointers[unresolved_pointer_index].mObject = existing_resource;
				}
				else
				{
					RTTI::Variant new_value = property.get_value(*resource);
					property.set_value(existing_resource, new_value);
				}
			}
		}

		return true;
	}

	void ResourceManagerService::backupObjects(const ExistingObjectMap& objects, ExistingObjectMap& backups)
	{
		for (auto kvp : objects)
		{
			Object* source = kvp.first;				// read object
			Object* target = kvp.second;			// object in ResourceMgr
			Object* copy = cloneObject(*target);
			backups.insert({ source, copy });		// Mapping from 'read object' to backup of file in ResourceMgr
		}
	}

	void ResourceManagerService::restoreObjects(ExistingObjectMap& objects, const ExistingObjectMap& backups)
	{
		for (auto kvp : objects)
		{
			Object* source = kvp.first;
			Object* target = kvp.second;

			ExistingObjectMap::const_iterator backup = backups.find(source);
			assert(backup != backups.end());

			copyObject(*(backup->second), *target);
		}
	}


	void ResourceManagerService::rollback(ExistingObjectMap& existingObjects, const ExistingObjectMap& backupObjects, const ObjectList& newObjects)
	{
		restoreObjects(existingObjects, backupObjects);
		
		for (auto kvp : backupObjects)
		{
			delete kvp.first;		// This is the existing object as read from disk
			delete kvp.second;		// This is the backup
		}

		for (Object* object : newObjects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource != nullptr)
			{
				if (findResource(resource->mID) != nullptr)
					removeResource(resource->mID);
			}

			delete object;
		}
	}

	bool ResourceManagerService::loadFile(const std::string& filename, nap::InitResult& initResult)
	{
		ObjectList read_objects;
		UnresolvedPointerList unresolved_pointers;
		JSONReader reader;

		// Read objects from disk into 'read_objects'. If this call fails, any objects that were 
		// successfully read are destructed, so no further action is required.
		if (!reader.Read(filename, read_objects, unresolved_pointers, initResult))
			return false;

		ExistingObjectMap existing_objects;		// Mapping from 'read object' to 'existing object in ResourceMgr'
		ObjectList new_objects;					// Objects not (yet) present in ResourceMgr
		ObjectList target_objects;				// List of all objects as they eventually will be in ResourceMgr
		splitObjects(read_objects, target_objects, existing_objects, new_objects);

		// First make clones of objects so that we can restore them if errors occurs
		ExistingObjectMap backup_objects;
		backupObjects(existing_objects, backup_objects);

		// Update attributes of objects already existing in ResourceMgr
		if (!updateExistingObjects(existing_objects, unresolved_pointers, initResult))
		{
			rollback(existing_objects, backup_objects, new_objects);
			return false;
		}

		// Add objects that were not yet present in ResourceMgr
		for (Object* object : new_objects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			addResource(resource->mID, resource);
		}

		// Resolve all unresolved pointers against the ResourceMgr
		for (const UnresolvedPointer& unresolved_pointer : unresolved_pointers)
		{
			nap::Resource* source_resource = rtti_cast<Resource>(unresolved_pointer.mObject);
			if (source_resource == nullptr)
				continue;

			Resource* target_resource = findResource(unresolved_pointer.mTargetID);
			if (!initResult.check(target_resource != nullptr, "Unable to resolve link to object %s from attribute %s", unresolved_pointer.mTargetID.c_str(), unresolved_pointer.mProperty.get_name().data()))
			{
				rollback(existing_objects, backup_objects, new_objects);
				return false;
			}

 			bool succeeded = unresolved_pointer.mProperty.set_value(unresolved_pointer.mObject, target_resource);
 			if (!initResult.check(succeeded, "Failed to resolve pointer"))
			{
				rollback(existing_objects, backup_objects, new_objects);
				return false;
			}
		}
		
		// Init all objects
		std::vector<Resource*> initted_objects;
		bool init_success = true;
		for (Object* object : target_objects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			initted_objects.push_back(resource);

			if (!resource->init(initResult))
			{
				init_success = false;
				break;
			}
		}

		// In case of error, rollback all modifications done by attempted init calls
		if (!init_success)
		{
			for (Resource* initted_object : initted_objects)
				initted_object->finish(Resource::EFinishMode::ROLLBACK);

			rollback(existing_objects, backup_objects, new_objects);
			return false;
		}

		// Everything successful, commit changes
		for (Object* object : target_objects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			resource->finish(Resource::EFinishMode::COMMIT);
		}

		mFilesToWatch.insert(toComparableFilename(filename));

		return true;
	}

	void ResourceManagerService::checkForFileChanges()
	{
		std::string modified_file;
		if (mDirectoryWatcher->update(modified_file))
		{
			if (mFilesToWatch.find(toComparableFilename(modified_file)) != mFilesToWatch.end())
			{
				nap::InitResult initResult;
				loadFile(modified_file, initResult);
			}
		}
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

	void ResourceManagerService::removeResource(const std::string& id)
	{
		assert(mResources.find(id) != mResources.end());
		mResources.erase(mResources.find(id));
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