#include "napgeneric.h"

#include <generic/utility.h>

using namespace nap::rtti;
using namespace nap::utility;

RTTITypeItem::RTTITypeItem(const nap::rtti::TypeInfo& type) : type(type)
{
	setText(type.get_name().data());
	setEditable(false);
	//    setForeground(softForeground());
	//    setBackground(softBackground());
	refresh();
}

void RTTITypeItem::refresh()
{
	for (const nap::rtti::TypeInfo& derived : type.get_derived_classes())
	{
		appendRow(new RTTITypeItem(derived));
	}
}

bool ResolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers)
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
