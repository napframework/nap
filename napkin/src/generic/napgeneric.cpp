#include "napgeneric.h"
#include <generic/utility.h>

using namespace nap::rtti;
using namespace nap::utility;

napkin::RTTITypeItem::RTTITypeItem(const nap::rtti::TypeInfo& type) : type(type)
{
	setText(type.get_name().data());
	setEditable(false);
	//    setForeground(getSoftForeground());
	//    setBackground(getSoftBackground());
	refresh();
}

void napkin::RTTITypeItem::refresh()
{
	for (const nap::rtti::TypeInfo& derived : type.get_derived_classes())
	{
		appendRow(new RTTITypeItem(derived));
	}
}

bool napkin::resolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers)
{
	std::map<std::string, RTTIObject*> objects_by_id;
	for (auto& object : objects)
		objects_by_id.insert({object->mID, object.get()});

	for (const UnresolvedPointer& unresolvedPointer : unresolvedPointers)
	{
		ResolvedRTTIPath resolved_path;
		if (!unresolvedPointer.mRTTIPath.resolve(unresolvedPointer.mObject, resolved_path))
			return false;

		auto pos = objects_by_id.find(unresolvedPointer.mTargetID);
		if (pos == objects_by_id.end())
			return false;

		if (!resolved_path.setValue(pos->second))
			return false;
	}

	return true;
}

nap::rtti::ObjectList napkin::topLevelObjects(const ObjectList& objects)
{
	// Pass 1: determine the set of all potential root objects
	std::vector<ObjectLink> all_object_links;
	ObjectSet allObjects;
	ObjectList objects_to_visit = objects;
	for (int index = 0; index < objects_to_visit.size(); ++index)
	{
		RTTIObject* object = objects_to_visit[index];
		allObjects.insert(object);

		// Find links for all objects
		std::vector<ObjectLink> links;
		findObjectLinks(*object, links);

		// Visit each links; the target of each link is a potential root object
		all_object_links.reserve(all_object_links.size() + links.size());
		objects_to_visit.reserve(objects_to_visit.size() + links.size());
		for (ObjectLink& link : links)
		{
			if (link.mTarget != nullptr && allObjects.find(link.mTarget) == allObjects.end())
				objects_to_visit.push_back(link.mTarget);

			all_object_links.push_back(link);
		}
	}

	// Pass 2: now that we know all potential root objects, build the list of actual root object
	// An object is a root object if it is not pointed to by an embedded pointer, or if it's pointed to by an embedded
	// pointer but the writer does not support embedded pointers
    ObjectList topLevelObjects; // RVO will take care of this
	topLevelObjects.reserve(allObjects.size());
	for (RTTIObject* object : allObjects)
	{
		bool is_embedded_object = false;

		// Scan through all links to figure out if any embedded pointer is pointing to this object.
		for (auto& link : all_object_links)
		{
			if (link.mTarget != object)
				continue;

			ResolvedRTTIPath resolved_path;
			link.mSourcePath.resolve(link.mSource, resolved_path);

			auto property = resolved_path.getProperty();
			if (hasFlag(property, EPropertyMetaData::Embedded))
			{
				is_embedded_object = true;
				break;
			}
		}

		// Only non-embedded objects can be roots
		if (!is_embedded_object)
			topLevelObjects.push_back(object);
	}

    // Probably no need to sort
//	// Pass 3: sort objects on type & ID to ensure files remain consistent after saving (for diffing and such)
//	std::sort(topLevelObjects.begin(), topLevelObjects.end(), [](RTTIObject* a, RTTIObject* b) {
//		if (a->get_type() == b->get_type())
//			return a->mID < b->mID;
//		else
//			return a->get_type().get_name().compare(b->get_type().get_name()) < 0;
//	});
    return topLevelObjects;
}
