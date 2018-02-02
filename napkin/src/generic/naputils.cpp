#include "naputils.h"

#include <QDir>

#include <component.h>
#include <nap/logger.h>
#include <entity.h>

#include "appcontext.h"

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



std::vector<rttr::type> napkin::getComponentTypes()
{
	std::vector<rttr::type> ret;
	nap::rtti::TypeInfo rootType = RTTI_OF(nap::Component);
	for (const nap::rtti::TypeInfo& derived : rootType.get_derived_classes())
	{
		if (derived.can_create_instance())
			ret.emplace_back(derived);
	}
	return ret;
}

std::vector<rttr::type> napkin::getResourceTypes()
{
	// TODO: Find a proper way to retrieve 'resource types' via RTTI
	std::vector<rttr::type> ret;
	rttr::type rootType = RTTI_OF(nap::rtti::RTTIObject);
	for (const rttr::type& derived : rootType.get_derived_classes())
	{
		if (derived.is_derived_from<nap::Component>())
			continue;
		if (derived.is_derived_from<nap::Entity>())
			continue;
		if (derived.is_derived_from<nap::ComponentInstance>())
			continue;
		if (!derived.can_create_instance())
			continue;

		ret.emplace_back(derived);
	}
	return ret;
}

nap::rtti::ResolvedRTTIPath napkin::resolve(const nap::rtti::RTTIObject& obj, nap::rtti::RTTIPath path)
{
	nap::rtti::ResolvedRTTIPath resolvedPath;
	path.resolve(&obj, resolvedPath);
	assert(resolvedPath.isValid());
	return resolvedPath;
}

nap::rtti::RTTIObject* napkin::getPointee(const PropertyPath& path)
{
	auto resolvedPath = path.resolve();
	auto value = resolvedPath.getValue();
	auto value_type = value.get_type();
	auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
	bool is_wrapper = wrapped_type != value_type;
	nap::rtti::RTTIObject* pointee = is_wrapper ? value.extract_wrapped_value().get_value<nap::rtti::RTTIObject*>()
												: value.get_value<nap::rtti::RTTIObject*>();
	return pointee;
}

bool napkin::setPointee(const nap::rtti::RTTIObject& obj, const nap::rtti::RTTIPath& path, const std::string& target)
{
	return false;
}

QString napkin::getAbsoluteResourcePath(const QString& relPath, const QString& reference)
{
	auto ref = getResourceReferencePath(reference);
	return QFileInfo(QString("%1/%2").arg(ref, relPath)).canonicalFilePath();
}

QString napkin::getRelativeResourcePath(const QString& absPath, const QString& reference)
{
	auto ref = getResourceReferencePath(reference);
	return QDir(ref).relativeFilePath(absPath);
}

QString napkin::getResourceReferencePath(const QString& reference)
{
	QString ref = reference;
	if (reference.isEmpty())
		ref = AppContext::get().getDocument()->getCurrentFilename();

	QFileInfo refinfo(ref);
	if (refinfo.isFile())
		ref = refinfo.path();

	return ref;
}

