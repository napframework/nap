#include "resourcemanager.h"

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <fstream>

RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{
	bool readObjects(rapidjson::Document& inDocument, ResourceManagerService& resourceManagerService, nap::InitResult& initResult)
	{
		using ObjectMap = std::unordered_map<std::string, nap::Object*>;
		ObjectMap objects;
		std::unordered_map<nap::ObjectLinkAttribute*, std::string> links_to_resolve;

		for (auto& object_pos = inDocument.MemberBegin(); object_pos < inDocument.MemberEnd(); ++object_pos)
		{
			const char* typeName = object_pos->name.GetString();
			RTTI::TypeInfo type_info = RTTI::TypeInfo::getByName(typeName);
			if (!initResult.check(type_info.isValid(), "Unknown object type %s encountered.", typeName))
				return false;

			if (!initResult.check(type_info.canCreateInstance(), "Unable to instantiate object of type %s.", typeName))
				return false;

			if (!initResult.check(type_info.isKindOf(RTTI_OF(nap::Resource)), "Unable to instantiate object %s. Class is not derived from Resource.", typeName))
				return false;

			nap::Resource* resource = type_info.createInstance<Resource>();

			for (auto& member_pos = object_pos->value.MemberBegin(); member_pos < object_pos->value.MemberEnd(); ++member_pos)
			{
				const char* attrName = member_pos->name.GetString();
				nap::Object* child = resource->getChild(attrName);
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
					links_to_resolve.insert({ (nap::ObjectLinkAttribute*)attribute, std::string(member_pos->value.GetString()) });
				}
			}

			std::string id = resource->mID.getValue();

			if (!initResult.check(!id.empty(), "Encountered object without ID"))
				return false;

			objects.insert(std::make_pair(id, resource));
			resourceManagerService.addResource(id, resource);
		}

		for (auto kvp : links_to_resolve)
		{
			ObjectMap::iterator target = objects.find(kvp.second);

			if (!initResult.check(target != objects.end(), "Unable to resolve link to object %s from attribute %s", kvp.second.c_str(), kvp.first->getName().c_str()))
				return false;

			kvp.first->setTarget(*target->second);
		}

		for (auto kvp : objects)
		{
			nap::Resource* resource = rtti_cast<nap::Resource>(kvp.second);
			if (!resource->init(initResult))
				return false;
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
		
		addResource(reso_unique_path, resource);
		
		return resource;
	}
}