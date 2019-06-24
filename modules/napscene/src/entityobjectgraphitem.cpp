#include "entityobjectgraphitem.h"
#include "component.h"
#include "rtti/rttiutilities.h"
#include "rtti/object.h"

namespace nap
{
	/**
	 * Creates a graph item.
	 * @param object Object to wrap in the item that is created.
	 */
	const EntityObjectGraphItem EntityObjectGraphItem::create(rtti::Object* object, const ObjectsByTypeMap& objectsByType, const ClonedResourceMap& clonedResourceMap)
	{
		EntityObjectGraphItem item;
		item.mType = EType::Object;
		item.mObject = object;
		item.mObjectsByType = &objectsByType;
		item.mClonedResourceMap = &clonedResourceMap;
			
		return item;
	}


	/**
	 * @return ID of the item. For objects, the ID is the object ID, for files, it is the filename.
	 */
	const std::string EntityObjectGraphItem::getID() const
	{
		assert(mType == EType::File || mType == EType::Object);

		if (mType == EType::File)
			return mFilename;
		else 
			return mObject->mID;
	}


	/**
	 * Performs rtti traversal of pointers to both files and objects.
	 * @param pointees Output parameter, contains all objects and files this object points to.
	 * @param errorState If false is returned, contains information about the error.
	 * @return true is succeeded, false otherwise.
	 */
	bool EntityObjectGraphItem::getPointees(std::vector<EntityObjectGraphItem>& pointees, utility::ErrorState& errorState) const
	{
		Component* component = rtti_cast<Component>(mObject);
		if (component != nullptr)
		{
			std::vector<rtti::TypeInfo> dependent_types;
			component->getDependentComponents(dependent_types);

			for (rtti::TypeInfo& type : dependent_types)
			{
				ObjectsByTypeMap::const_iterator dependent_component = mObjectsByType->find(type);
				if (dependent_component == mObjectsByType->end())
					return true;

				const std::vector<rtti::Object*> components = dependent_component->second;
				for (rtti::Object* component : components)
				{
					EntityObjectGraphItem item;
					item.mType				= EType::Object;
					item.mObject			= component;
					item.mObjectsByType		= mObjectsByType;
					item.mClonedResourceMap = mClonedResourceMap;
					pointees.push_back(item);
				}
			}
		}

		std::vector<rtti::ObjectLink> object_links;
		rtti::findObjectLinks(*mObject, object_links);

		for (const rtti::ObjectLink& link : object_links)
		{
			if (link.mTarget == nullptr)
				continue;

			EntityObjectGraphItem item;
			item.mType				= EType::Object;
			item.mObject			= link.mTarget;
			item.mObjectsByType		= mObjectsByType;
			item.mClonedResourceMap = mClonedResourceMap;
			pointees.push_back(item);
		}

		std::vector<std::string> file_links;
		rtti::findFileLinks(*mObject, file_links);

		for (std::string& filename : file_links)
		{
			EntityObjectGraphItem item;
			item.mType					= EType::File;
			item.mFilename				= filename;
			item.mObjectsByType			= mObjectsByType;
			item.mClonedResourceMap		= mClonedResourceMap;
			pointees.push_back(item);
		}

		size_t pointees_size = pointees.size();
		for (int i = 0; i != pointees_size; ++i)
		{
			EntityObjectGraphItem& item = pointees[i];
			if (item.mType == EType::Object)
			{
				ClonedResourceMap::const_iterator pos = mClonedResourceMap->find(item.mObject);
				if (pos != item.mClonedResourceMap->end())
				{
					for (rtti::Object* clonedResource : pos->second)
					{
						EntityObjectGraphItem cloned_item;
						cloned_item.mType				= EType::Object;
						cloned_item.mObject				= clonedResource;
						cloned_item.mObjectsByType		= mObjectsByType;
						cloned_item.mClonedResourceMap	= mClonedResourceMap;
						pointees.push_back(cloned_item);
					}
				}
			}
		}
			
		return true;
	}
}