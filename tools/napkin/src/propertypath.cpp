/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "propertypath.h"
#include "naputils.h"
#include "typeconversion.h"

#include <rtti/linkresolver.h>
#include <rtti/defaultlinkresolver.h>
#include <appcontext.h>
#include <stack>
#include <cctype>
#include <mathutils.h>
#include <color.h>
#include <componentptr.h>
#include <entityptr.h>

using namespace nap::rtti;
using namespace napkin;

NameIndex::NameIndex(const std::string& nameIndex)
{
	nameAndIndex(nameIndex, mID, mIndex);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PropertyPath::PropertyPath(Object& obj, Document& doc) : mDocument(&doc), mObject(&obj)
{
	mObjectPath.emplace_back(obj.mID);
}


PropertyPath::PropertyPath(const std::string& abspath, Document& doc) : mDocument(&doc)
{
	auto path_parts = nap::utility::splitString(abspath, '@');
	auto path_elems = nap::utility::splitString(path_parts[0], '/');

	// Construct object path
	for (auto path_elem: path_elems)
	{
		if(!path_elem.empty())
			mObjectPath.emplace_back(path_elem);
	}

	// Construct property path
	if (path_parts.size() > 1)
	{
		for (auto propElm : nap::utility::splitString(path_parts[1], '/'))
			mPropertyPath.emplace_back(propElm);
	}
}


PropertyPath::PropertyPath(const std::string& abspath, const std::string& proppath, Document& doc)  : mDocument(&doc)
{
	// Construct object path
	auto abs_elems = nap::utility::splitString(abspath, '/');
	for (const auto& elem : abs_elems)
	{
		if (!elem.empty())
			mObjectPath.emplace_back(elem);
	}

	// Construct property path
	auto prop_elems = nap::utility::splitString(proppath, '/');
	for (const auto& elem : prop_elems)
		mPropertyPath.emplace_back(elem);
}


PropertyPath::PropertyPath(const PPath& abspath, Document& doc)
		: mObjectPath(abspath), mDocument(&doc)
{ }


PropertyPath::PropertyPath(const PPath& absPath, const PPath& propPath, Document& doc)
		: mObjectPath(absPath), mPropertyPath(propPath), mDocument(&doc)
{ }


PropertyPath::PropertyPath(Object& obj, const Path& path, Document& doc) : mDocument(&doc), mObject(&obj)
{
	mObjectPath.emplace_back(NameIndex(obj.mID));
	mPropertyPath.emplace_back(path.toString());
}


PropertyPath::PropertyPath(nap::rtti::Object& obj, rttr::property prop, Document& doc) : mDocument(&doc)
{
	mObjectPath.emplace_back(obj.mID);
	mPropertyPath.emplace_back(std::string(prop.get_name().data()));
}


const std::string PropertyPath::getName() const
{
	return hasProperty() ? getProperty().get_name().data() : getObject()->mID;
}


nap::ComponentInstanceProperties* PropertyPath::getInstanceProperties() const
{
	// check if we have a root and are part of the scene
	auto root_entitiy = getRootEntity();
	if (root_entitiy == nullptr || root_entitiy->mInstanceProperties.empty())
		return nullptr;

	// Check if we have a match
	auto comp_instance_path = getComponentInstancePath();
	for (nap::ComponentInstanceProperties& instProp : root_entitiy->mInstanceProperties)
	{
		// TODO: This means the component ptr wasn't resolved, probably because of a name change
		// TODO: Make sure to patch ComponentInstanceProperties
		if(instProp.mTargetComponent.get() == nullptr)
			continue;

		if (isComponentInstancePathEqual(instProp.mTargetComponent.getInstancePath(), comp_instance_path))
			return &instProp;
	}
	return nullptr;
}


nap::ComponentInstanceProperties& PropertyPath::getOrCreateInstanceProps()
{
	// Get existing instance property overrides
	assert(isInstanceProperty());
	auto overrides = getInstanceProperties();

	// No instance properties, create a new set
	if (overrides == nullptr)
	{
		auto root_entity = getRootEntity();
		overrides = &root_entity->mInstanceProperties.emplace_back();

		std::string target_path = getComponentInstancePath();
		auto* component = rtti_cast<nap::Component>(getObject());
		assert(!target_path.empty() && component != nullptr);
		overrides->mTargetComponent.assign(target_path, component);
	}
	return *overrides;
}


std::string PropertyPath::getComponentInstancePath() const
{
	// Check if we're a component
	assert(getRootEntity() != nullptr);
	auto* comp_object = rtti_cast<nap::Component>(getObject());
	if (comp_object == nullptr)
		return {};

	// Create path to component from root entity
	assert(mObjectPath.size() > 2);
	std::vector<std::string> component_path(mObjectPath.begin() + 2, mObjectPath.begin() + mObjectPath.size());
	return "./" + nap::utility::joinString(component_path, "/");
}


nap::RootEntity* PropertyPath::getRootEntity() const
{
	// Find root if not queried before, note that a path doesn't have to have a root entity.
	// This is the case when editing a path of a regular resource, not an entity instance.
	if (mObjectPath.size() > 1 && !mRootQueried)
	{
		// Must have 2 objects, of which first the scene
		assert(mDocument != nullptr);
		if (rtti_cast<nap::Scene>(mDocument->getObject(mObjectPath[0].mID)) != nullptr)
		{
			auto* scene = static_cast<nap::Scene*>(mDocument->getObject(mObjectPath[0].mID));
			auto* entity = rtti_cast<nap::Entity>(mDocument->getObject(mObjectPath[1].mID));
			assert(entity != nullptr);

			// Find entity matching index in scene
			auto entity_idx = mObjectPath[1].mIndex; int idx = 0;
			for (auto& scene_entity : scene->mEntities)
			{
				if (scene_entity.mEntity.get() == entity && entity_idx == idx++)
				{
					mRootEntity = &scene_entity;
					break;
				}
			}
		}
		mRootQueried = true;
	}
	return mRootEntity;
}


nap::TargetAttribute* PropertyPath::getTargetAttribute() const
{
	nap::TargetAttribute* target = nullptr;
	auto overrides = getInstanceProperties();
	if (overrides != nullptr)
	{
		auto prop_path = propPathStr();
		auto it = std::find_if(overrides->mTargetAttributes.begin(), overrides->mTargetAttributes.end(), [&prop_path](const auto& attr)
			{
				return attr.mPath == prop_path;
			});
		target = it != overrides->mTargetAttributes.end() ? &(*it) : nullptr;
	}
	return target;
}


nap::TargetAttribute& PropertyPath::getOrCreateTargetAttribute()
{
	// Find in existing instance properties
	assert(isInstanceProperty());
	auto target_attr = getTargetAttribute();

	// Create if it doesn't exist
	if (target_attr == nullptr)
	{
		// Create target attribute
		auto& instance_properties = getOrCreateInstanceProps();
		target_attr = &instance_properties.mTargetAttributes.emplace_back();
		target_attr->mPath = propPathStr();
	}
	return *target_attr;
}


rttr::variant PropertyPath::getValue() const
{
	if (isInstanceProperty() && isOverridden())
	{
		auto target_attr = getTargetAttribute();
		if (target_attr)
		{
			return getInstancePropertyValue(*target_attr->mValue.get());
		}
	}
	return resolve().getValue();
}


rttr::variant napkin::PropertyPath::patchValue(const rttr::variant& value) const
{
	// Only patch component and entity ptr paths
	auto prop_type = getType();
	if (!prop_type.is_derived_from(RTTI_OF(nap::ComponentPtrBase)) &&
		!prop_type.is_derived_from(RTTI_OF(nap::EntityPtr)))
	{
		return value;
	}

	// Extract pointer from value
	auto* target_object = value.get_type().is_wrapper() ?
		value.get_wrapped_value<nap::rtti::Object*>() : value.get_value<nap::rtti::Object*>();

	// Construct path to new pointer
	// Note that it's allowed to invalidate the link by assigning it nothing (nullptr & empty path)
	std::string path = target_object != nullptr ?
		mDocument->relativeObjectPath(*getObject(), *target_object) : "";

	// Assign the new value to the entity or component pointer (note that we're modifying a copy)
	auto patched_ptr = getValue();
	rttr::method assign_method = nap::rtti::findMethodRecursive(patched_ptr.get_type(), nap::rtti::method::assign);
	assert(assign_method.is_valid());
	assign_method.invoke(patched_ptr, path, target_object);

	// Return it
	return patched_ptr;
}


bool PropertyPath::setValue(rttr::variant new_value)
{
	// Resolve path to property
	auto resolved_path = resolve();
	auto resource_value = resolved_path.getValue();

	// New value doesn't override resource value
	if (resource_value == new_value)
	{
		// Delete instance override when currently set
		auto target_attr = getTargetAttribute();
		if (target_attr != nullptr)
		{
			rttr::variant val = target_attr->mValue.get();
			removeInstanceValue(target_attr, val);
		}
		return true;
	}

	// Set value directly when we're not dealing with an instance override
	// We patch the value to ensure component and entity ptr paths are also updated
	auto patched_value = patchValue(new_value);
	if (!isInstanceProperty())
		return resolved_path.setValue(patched_value);

	//////////////////////////////////////////////////////////////////////////
	// TODO: Improve handling of instance properties!!!
	// The current implementation is shaky at best. It works, but that's about it. What to do?
	// Properly handle all types of nap::InstancePropertyValue! (no more exceptions)
	// Properly implement callbacks, they're too scattered and hard to trace!
	// Strengthen and simplify the entire model!
	//////////////////////////////////////////////////////////////////////////

	// Get instance property (as target). Create one if it doesn't exist.
	// If it does exist: discard instance property value if the provided value is the same as the original
	auto target_attr = getTargetAttribute();
	if (target_attr == nullptr)
	{
		nap::InstancePropertyValue* new_value = createInstanceProperty(getType(), *getDocument());
		if (new_value == nullptr)
			return false;

		target_attr = &getOrCreateTargetAttribute();
		target_attr->mValue = new_value;
	}

	// Set instance property value
	rttr::variant val = target_attr->mValue.get();
	return setInstancePropertyValue(val, patched_value);
}


void PropertyPath::removeInstanceValue(const nap::TargetAttribute* targetAttr, rttr::variant& val) const
{
	// TODO: Move this logic to napkin::Document and implement valueChanged signal for various properties.
	// This allows the various models to respond appropriately -> only document should emit signals.

	// remove from target attributes list
	auto instProps = getInstanceProperties();
	auto& attrs = instProps->mTargetAttributes;
	auto filter = [&](const nap::TargetAttribute& attr) { return &attr == targetAttr; };
	attrs.erase(std::remove_if(attrs.begin(), attrs.end(), filter), attrs.end());

	// remove attributes list if necessary
	if (attrs.empty())
	{
		auto flt = [&](const nap::ComponentInstanceProperties& instProp) { return &instProp == instProps; };
		auto& rootInstProps = this->getRootEntity()->mInstanceProperties;
		rootInstProps.erase(std::remove_if(rootInstProps.begin(), rootInstProps.end(), flt), rootInstProps.end());
	}

	// Remove from object list
	removeInstanceProperty(val, *getDocument());
	assert(mDocument != nullptr);
}


Object* PropertyPath::getPointee() const
{
	if (!isPointer())
		return nullptr;

	auto value = getValue();
	auto type = value.get_type();
	auto wrapped_type = type.is_wrapper() ? type.get_wrapped_type() : type;
	return wrapped_type != type ? 
		value.extract_wrapped_value().get_value<nap::rtti::Object*>() :
		value.get_value<nap::rtti::Object*>();
}


PropertyPath PropertyPath::getParent() const
{
	assert(mDocument != nullptr);
	if (hasProperty())
	{
		if (mPropertyPath.size() > 1)
			return { mObjectPath, PPath(mPropertyPath.begin(), mPropertyPath.begin() + mPropertyPath.size() - 1), *mDocument };

		if (mPropertyPath.size() == 1)
			return { mObjectPath, *mDocument };
	}

	if (mObjectPath.size() >= 2)
		return { PPath(mObjectPath.begin(), mObjectPath.begin() + mObjectPath.size() - 1), *mDocument };

	// Invalid parent
	return { };
}


rttr::property PropertyPath::getProperty() const
{
	return resolve().getProperty();
}


rttr::type PropertyPath::getType() const
{
	if (!getObject())
		return rttr::type::empty();

	if (!hasProperty())
		return getObject()->get_type();

	Variant value = resolve().getValue();
	return value.get_type();
}


ResolvedPath PropertyPath::resolve() const
{
	auto path = Path::fromString(propPathStr());
	ResolvedPath resolvedPath;
	path.resolve(getObject(), resolvedPath);
	return resolvedPath;
}


rttr::type PropertyPath::getArrayElementType() const
{
	ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_valid());
	if (!array.is_array())
		return rttr::type::empty();

	VariantArray array_view = array.create_array_view();
	auto elmtype = array_view.get_rank_type(1);
	return elmtype.is_wrapper() ? elmtype.get_wrapped_type() : elmtype;
}


size_t PropertyPath::getArrayLength() const
{
	ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_valid());

	VariantArray array_view = array.create_array_view();
	return array_view.get_size();
}


PropertyPath PropertyPath::getArrayElement(size_t index) const
{
	if (!isArray())
		return PropertyPath();

	PPath p = mPropertyPath;
	p.emplace_back(std::to_string(index));
	assert(mDocument != nullptr);
	return { mObjectPath, p, *mDocument };
}


bool napkin::PropertyPath::getArrayEditable() const
{
	assert(isArray());
	ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());
	Variant array = resolved_path.getValue();
	assert(array.is_valid());
	return array.create_array_view().is_dynamic(); 
}


bool napkin::PropertyPath::referencesObject(const std::string& name)
{
	for (const auto& idx_name : mObjectPath)
	{
		if (idx_name.mID == name)
			return true;
	}
	return false;
}


std::string PropertyPath::toString() const
{
	return hasProperty() ? propPathStr() + "@" + objectPathStr() : objectPathStr();
}


bool PropertyPath::isInstanceProperty() const
{
	return this->getRootEntity() != nullptr;
}


PropertyPath PropertyPath::getChild(const std::string& name) const
{
	assert(mDocument != nullptr);
	return {objectPathStr(), propPathStr() + "/" + name, *mDocument};
}


nap::rtti::Object* PropertyPath::getObject() const
{
	if (mObject == nullptr && !mObjectPath.empty())
	{
		assert(mDocument != nullptr);
		mObject = mDocument->getObject(mObjectPath.back().mID);
	}
	return mObject;
}


Path PropertyPath::getPath() const
{
	return Path::fromString(propPathStr());
}


rttr::type PropertyPath::getWrappedType() const
{
	const auto& type = getType();
	return type.is_wrapper() ? type.get_wrapped_type() : type;
}


bool PropertyPath::isOverridden() const
{
	return hasProperty() ? getTargetAttribute() != nullptr : false;
}


void PropertyPath::removeOverride()
{
	auto target_attr = getTargetAttribute();
	assert(target_attr != nullptr);
	rttr::variant val = target_attr->mValue.get();
	removeInstanceValue(target_attr, val);
}


bool PropertyPath::hasProperty() const
{
	return !mPropertyPath.empty();
}


bool PropertyPath::isValid() const
{
	// If it's a property, resolve and validate
	return hasProperty() ? resolve().isValid() :
		getObject() != nullptr;
}


bool PropertyPath::operator==(const PropertyPath& other) const
{
	return objectPathStr() == other.objectPathStr() && propPathStr() == other.propPathStr();
}


bool PropertyPath::isArray() const
{
	return getType().is_array();
}


bool PropertyPath::isPointer() const
{
	return isArray() ? getArrayElementType().is_pointer() : getWrappedType().is_pointer();
}


bool PropertyPath::isEmbeddedPointer() const
{
	return isPointer() ? nap::rtti::hasFlag(getProperty(), EPropertyMetaData::Embedded) : false;
}


bool PropertyPath::isNonEmbeddedPointer() const
{
	return isPointer() ? !nap::rtti::hasFlag(getProperty(), EPropertyMetaData::Embedded) : false;
}


bool PropertyPath::isEnum() const
{
	return getWrappedType().is_enumeration();
}


bool PropertyPath::isColor() const
{
	return getWrappedType().is_derived_from(RTTI_OF(nap::BaseColor));
}


void PropertyPath::iterateChildren(std::function<bool(const PropertyPath&)> visitor, int flags) const
{
	if (!getObject())
		return;

	rttr::type type = getType();
	if (isArray())
	{
		iterateArrayElements(visitor, flags);
		return;
	}
	else if (type.is_associative_container())
	{
		return;
	}
	else if (isPointer())
	{
		iteratePointerProperties(visitor, flags);
	}
	else if (nap::rtti::isPrimitive(type))
	{
		return;
	}
	else // compound property
	{
		iterateChildrenProperties(visitor, flags);
	}

}

std::vector<PropertyPath> PropertyPath::getChildren(int flags) const
{
	std::vector<PropertyPath> children;

	iterateChildren([&children](const auto& path)
					{
						children.emplace_back(path);
						return true;
					}, flags);

	return children;
}

void PropertyPath::iterateProperties(PropertyVisitor visitor, int flags) const
{
	if (!getObject())
		return;

	for (rttr::property prop : getObject()->get_type().get_properties())
	{
		PPath propPath;
		propPath.emplace_back(std::string(prop.get_name().data()));
		assert(mDocument != nullptr);
		PropertyPath path(mObjectPath, propPath, *mDocument);

		if (!visitor(path))
			return;

		if (flags & IterFlag::Resursive)
			path.iterateChildren(visitor, flags);
	}
}


std::vector<PropertyPath> PropertyPath::getProperties(int flags) const
{
	std::vector<PropertyPath> props;

	iterateProperties([&props](const auto& path)
					  {
						  props.emplace_back(path);
						  return true;
					  }, flags);

	return props;
}


int PropertyPath::getInstanceChildEntityIndex() const
{
	if (mObjectPath.empty())
		return -1;

	return mObjectPath.back().mIndex;
}


int PropertyPath::getEntityIndex() const
{
	auto parent = getParent();
	assert(parent.isValid());
	assert(parent.getType().is_derived_from<nap::Entity>());
	auto parent_entity = rtti_cast<nap::Entity>(parent.getObject());
	assert(parent_entity);

	int found_id = 0;
	int instance_idx = getInstanceChildEntityIndex();
	for (int i = 0, len = static_cast<int>(parent_entity->mChildren.size()); i < len; i++)
	{
		auto current_child = parent_entity->mChildren[i];
		if (getObject()->mID == current_child->mID)
		{
			if (found_id != instance_idx)
			{
				found_id++;
				continue;
			}
			return i;
		}
	}
	assert(false);
	return -1;
}


void PropertyPath::iterateArrayElements(PropertyVisitor visitor, int flags) const
{
	auto value = getValue();
	auto array = value.create_array_view();

	for (int i = 0; i < array.get_size(); i++)
	{
		PPath p = mPropertyPath;
		p.emplace_back(std::to_string(i));
		assert(mDocument != nullptr);
		PropertyPath childPath(mObjectPath, p, *mDocument);

		if (!visitor(childPath))
			return;

		if (flags & IterFlag::Resursive)
			childPath.iterateChildren(visitor, flags);
	}
}


void PropertyPath::iterateChildrenProperties(PropertyVisitor visitor, int flags) const
{
	for (auto childProp : getType().get_properties())
	{
		PPath p = mPropertyPath;
		p.emplace_back(std::string(childProp.get_name().data()));
		assert(mDocument != nullptr);
		PropertyPath childPath(mObjectPath, p, *mDocument);

		if (!visitor(childPath))
			return;

		if (flags & IterFlag::Resursive)
			childPath.iterateChildren(visitor, flags);
	}
}


void PropertyPath::iteratePointerProperties(PropertyVisitor visitor, int flags) const
{
	if (isEmbeddedPointer())
	{
		if (!(flags & IterFlag::FollowEmbeddedPointers))
			return;
	}
	else
	{
		if (!(flags & IterFlag::FollowPointers))
			return;
	}

	// prune here if there is no pointer value
	auto pointee = getPointee();
	if (pointee == nullptr)
		return;

	for (auto childprop : pointee->get_type().get_properties())
	{
		auto childValue = childprop.get_value(pointee);
		std::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		// This path points to the pointee
		PPath op = { pointee->mID };
		PPath pp = { name };
		assert(mDocument != nullptr);
		PropertyPath childPath(op, pp, *mDocument);

		if (!visitor(childPath))
			return;
	}
}


std::string PropertyPath::objectPathStr() const
{
	std::string obj_path;
	for (const auto& elm : mObjectPath)
		obj_path += nap::utility::stringFormat("/%s", elm.toString().c_str());
	return obj_path;
}


std::string PropertyPath::propPathStr() const
{
	std::string pro_path;
	for (auto i = 0; i < mPropertyPath.size(); i++)
	{
		if (i > 0)
			pro_path += "/";
		pro_path += mPropertyPath[i];
	}
	return pro_path;
}


void PropertyPath::updateObjectName(const std::string& oldName, const std::string& newName)
{
	for (auto& nameIdx : mObjectPath)
	{
		if (nameIdx.mID == oldName)
			nameIdx.mID = newName;
	}
}
