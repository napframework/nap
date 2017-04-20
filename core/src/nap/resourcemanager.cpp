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


namespace nap
{
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

	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(new DirectoryWatcher())
	{
	}

	using ResourceMap = std::unordered_map<std::string, nap::Resource*>;
	using LinkMap = std::unordered_map<nap::ObjectLinkAttribute*, std::string>;

	bool readObject(rapidjson::Document::MemberIterator& object, nap::Resource*& newObject, LinkMap& linkMap, nap::InitResult& initResult)
	{
		const char* typeName = object->name.GetString();
		RTTI::TypeInfo type_info = RTTI::TypeInfo::getByName(typeName);
		if (!initResult.check(type_info.isValid(), "Unknown object type %s encountered.", typeName))
			return false;

		if (!initResult.check(type_info.canCreateInstance(), "Unable to instantiate object of type %s.", typeName))
			return false;

		if (!initResult.check(type_info.isKindOf(RTTI_OF(nap::Resource)), "Unable to instantiate object %s. Class is not derived from Resource.", typeName))
			return false;

		newObject = type_info.createInstance<Resource>();

		for (auto& member_pos = object->value.MemberBegin(); member_pos < object->value.MemberEnd(); ++member_pos)
		{
			const char* attrName = member_pos->name.GetString();
			nap::Object* child = newObject->getChild(attrName);
			nap::AttributeBase* attribute = rtti_cast<nap::AttributeBase>(child);

			if (attribute == nullptr)
				continue;

			if (attribute->getTypeInfo().isKindOf(RTTI_OF(nap::Attribute<std::string>)))
			{
				((nap::Attribute<std::string>*)attribute)->setValue(member_pos->value.GetString());
			}
			else if (attribute->getTypeInfo().isKindOf(RTTI_OF(nap::NumericAttribute<int>)))
			{
				((nap::NumericAttribute<int>*)attribute)->setValue(member_pos->value.GetInt());
			}
			else if (attribute->getTypeInfo().isKindOf(RTTI_OF(nap::ObjectLinkAttribute)))
			{
				linkMap.insert({ (nap::ObjectLinkAttribute*)attribute, std::string(member_pos->value.GetString()) });
			}
		}

		std::string id = newObject->mID.getValue();

		if (!initResult.check(!id.empty(), "Encountered object without ID"))
		{
			delete newObject;
			return false;
		}

		return true;
	}


	bool readObjects(rapidjson::Document& document, ResourceMap& objectMap, LinkMap& linkMap, nap::InitResult& initResult)
	{
		objectMap.clear();
		linkMap.clear();

		for (auto& object_pos = document.MemberBegin(); object_pos < document.MemberEnd(); ++object_pos)
		{
			Resource* new_object = nullptr;
			if (readObject(object_pos, new_object, linkMap, initResult))
			{
				objectMap.insert({ new_object->mID.getValue(), new_object });
			}
			else
			{
				for (auto kvp : objectMap)
					delete kvp.second;

				objectMap.clear();
				linkMap.clear();

				return false;
			}
		}
		return true;
	}

	void splitObjects(ResourceManagerService& resourceManagerService, const ResourceMap& sourceObjectMap, ResourceMap& targetObjectMap, ResourceMap& existingObjectMap, ResourceMap& newObjectMap)
	{
		for (auto kvp : sourceObjectMap)
		{
			Resource* resource = kvp.second;
			std::string id = resource->mID.getValue();

			Resource* existing_resource = resourceManagerService.findResource(id);
			if (existing_resource == nullptr)
			{
				newObjectMap.insert({ id, resource });
				targetObjectMap.insert({ id, resource });
			}
			else
			{
				existingObjectMap.insert({ id, resource });
				targetObjectMap.insert({ id, existing_resource });
			}
		}
	}

	bool updateExistingObjects(ResourceManagerService& resourceManagerService, const ResourceMap& existingObjectMap, LinkMap& linkMap, InitResult& initResult)
	{
		for (auto kvp : existingObjectMap)
		{
			Resource* resource = kvp.second;
			std::string id = resource->mID.getValue();

			Resource* existing_resource = resourceManagerService.findResource(id);
			if (existing_resource != nullptr)
			{
				if (!initResult.check(existing_resource->getTypeInfo() == resource->getTypeInfo(), "Unable to update object, different types"))		// todo: actually support this properly
					return false;

				for (nap::Object* child : resource->getChildren())
				{
					nap::AttributeBase* child_attribute = rtti_cast<nap::AttributeBase>(child);
					if (child_attribute != nullptr)
					{
						nap::Object* dest_child = existing_resource->getChild(child->getName());
						nap::AttributeBase* dest_child_attribute = rtti_cast<nap::AttributeBase>(dest_child);

						if (dest_child_attribute->getTypeInfo().isKindOf(RTTI_OF(nap::Attribute<std::string>)))
						{
							((nap::Attribute<std::string>*)dest_child_attribute)->setValue(((nap::Attribute<std::string>*)child_attribute)->getValue());
						}
						else if (dest_child_attribute->getTypeInfo().isKindOf(RTTI_OF(nap::NumericAttribute<int>)))
						{
							((nap::NumericAttribute<int>*)dest_child_attribute)->setValue(((nap::NumericAttribute<int>*)child_attribute)->getValue());
						}
						else if (dest_child_attribute->getTypeInfo().isKindOf(RTTI_OF(nap::ObjectLinkAttribute)))
						{
							auto existing = linkMap.find((nap::ObjectLinkAttribute*)child_attribute);
							linkMap.insert({ (nap::ObjectLinkAttribute*)dest_child_attribute, existing->second });			// TODO: solve this ugly link rerouting
							linkMap.erase(existing);
						}
					}
				}
			}
		}

		return true;
	}

	bool readObjects(rapidjson::Document& document, ResourceManagerService& resourceManagerService, nap::InitResult& initResult)
	{
		ResourceMap sourceObjectMap;
		ResourceMap objectMap;
		LinkMap linkMap;

		if (!readObjects(document, sourceObjectMap, linkMap, initResult))
			return false;

		ResourceMap targetObjectMap;
		ResourceMap existingObjectMap;
		ResourceMap newObjectMap;
		splitObjects(resourceManagerService, sourceObjectMap, targetObjectMap, existingObjectMap, newObjectMap);

		if (!updateExistingObjects(resourceManagerService, existingObjectMap, linkMap, initResult))
		{
			for (auto kvp : sourceObjectMap)
				delete kvp.second;

			// TODO: on failure, attributes of objects are in half updated state. fix

			return false;
		}

		for (auto kvp : newObjectMap)
		{
			nap::Resource* resource = kvp.second;
			resourceManagerService.addResource(resource->mID.getValue(), resource);
		}

		for (auto kvp : linkMap)
		{
			Resource* target = resourceManagerService.findResource(kvp.second);
			if (!initResult.check(target != nullptr, "Unable to resolve link to object %s from attribute %s", kvp.second.c_str(), kvp.first->getName().c_str()))
				return false;

			// TODO: on failure, attributes of objects are in half updated state + new object must be removed. fix

			kvp.first->setTarget(*target);
		}

		std::vector<Resource*> initted_objects;
		bool init_success = true;
		for (auto kvp : targetObjectMap)
		{
			nap::Resource* resource = kvp.second;
			if (!resource->init(initResult))
			{
				init_success = false;
				break;
			}

			initted_objects.push_back(resource);
		}

		if (!init_success)
		{
			for (Resource* initted_object : initted_objects)
				initted_object->finish(Resource::EFinishMode::ROLLBACK);

			// TODO: on failure, attributes of objects are in half updated state + new object must be removed. fix
			return false;
		}
		
		for (auto kvp : targetObjectMap)
		{
			kvp.second->finish(Resource::EFinishMode::COMMIT);
		}

		return true;
	}

	bool ResourceManagerService::loadFile(const std::string& filename, nap::InitResult& initResult)
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

		// Parse document
		rapidjson::Document doc;
		rapidjson::ParseResult parse_result = doc.ParseInsitu((char*)buffer.c_str());

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
	
		if (!readObjects(doc, *this, initResult))
			return false;

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

	Resource* ResourceManagerService::createResource(const RTTI::TypeInfo& type)
	{
		if (!type.isKindOf(RTTI_OF(Resource)))
		{
			nap::Logger::warn("unable to create resource of type: %s", type.getName().c_str());
			return nullptr;
		}

		if (!type.canCreateInstance())
		{
			nap::Logger::warn("can't create resource instance of type: %s", type.getName().c_str());
			return nullptr;
		}

		// Create instance of resource
		Resource* resource = type.createInstance<Resource>();

		// Construct path
		std::string type_name = type.getName().c_str();
		std::string reso_path = stringFormat("resource::%s", type_name.c_str());
		std::string reso_unique_path = reso_path;
		int idx = 0;
		while (mResources.find(reso_unique_path) != mResources.end())
		{
			++idx;
			reso_unique_path = stringFormat("%s_%d", reso_path.c_str(), idx);
		}

		resource->mID.setValue(reso_unique_path);
		addResource(reso_unique_path, resource);
		
		return resource;
	}
}