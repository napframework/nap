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

using namespace nap::rtti;
using namespace napkin;

NameIndex::NameIndex(const std::string& nameIndex)
{
	nameAndIndex(nameIndex, mID, mIndex);
}

std::string NameIndex::toString() const
{
	if (mIndex < 0)
		return mID;
	return mID + ":" + std::to_string(mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PropertyPath::PropertyPath(Object& obj, Document& doc) 
	: mDocument(&doc)
{
	mObjectPath.emplace_back(obj.mID);
}

PropertyPath::PropertyPath(const std::string& abspath, Document& doc) 
	: mDocument(&doc)
{
	auto pathParts = nap::utility::splitString(abspath, '@');
	for (auto pathElm : nap::utility::splitString(pathParts[0], '/'))
		if (!pathElm.empty())
			mObjectPath.emplace_back(pathElm);

	if (pathParts.size() > 1)
		for (auto propElm : nap::utility::splitString(pathParts[1], '/'))
			mPropertyPath.emplace_back(propElm);
}

PropertyPath::PropertyPath(const std::string& abspath, const std::string& proppath, Document& doc) 
	: mDocument(&doc)
{
	for (const auto& pathElm : nap::utility::splitString(abspath, '/'))
		if (!pathElm.empty())
			mObjectPath.emplace_back(pathElm);

	for (const auto& propElm : nap::utility::splitString(proppath, '/'))
		mPropertyPath.emplace_back(propElm);
}


PropertyPath::PropertyPath(const PPath& abspath, Document& doc)
		: mObjectPath(abspath), mDocument(&doc)
{
}

PropertyPath::PropertyPath(const PPath& absPath, const PPath& propPath, Document& doc)
		: mObjectPath(absPath), mPropertyPath(propPath), mDocument(&doc)
{
}


PropertyPath::PropertyPath(Object& obj, const Path& path, Document& doc) : mDocument(&doc)
{
	auto id = obj.mID;
	mObjectPath.emplace_back(NameIndex(id));
	mPropertyPath.emplace_back(path.toString());
}


PropertyPath::PropertyPath(nap::rtti::Object& obj, rttr::property prop, Document& doc) : mDocument(&doc)
{
	mObjectPath.emplace_back(obj.mID);
	mPropertyPath.emplace_back(std::string(prop.get_name().data()));
}

PropertyPath::~PropertyPath()
{ }

const std::string PropertyPath::getName() const
{
	if (hasProperty())
		return getProperty().get_name().data();
	return getObject()->mID;
}

nap::ComponentInstanceProperties* PropertyPath::instanceProps() const
{
	if (!isInstanceProperty())
		return nullptr;

	auto pathstr = propPathStr();

	// find instanceproperties
	auto rootEntity = getRootEntity();
	if (rootEntity->mInstanceProperties.empty())
		return nullptr;

	auto compInstPath = getComponentInstancePath();
	for (nap::ComponentInstanceProperties& instProp : rootEntity->mInstanceProperties)
	{
		if (isComponentInstancePathEqual(*rootEntity,
										 *instProp.mTargetComponent.get(),
										 instProp.mTargetComponent.getInstancePath(),
										 compInstPath))
			return &instProp;
	}
	return nullptr;
}

nap::ComponentInstanceProperties& PropertyPath::getOrCreateInstanceProps()
{
	assert(isInstanceProperty());

	auto props_ = instanceProps();
	if (props_)
		return *props_;

	// No instance properties, create a new set
	auto rootEntity = getRootEntity();
	auto idx = rootEntity->mInstanceProperties.size();
	rootEntity->mInstanceProperties.emplace_back();

	std::string targetID = getComponentInstancePath();

	rootEntity->mInstanceProperties.at(idx).mTargetComponent.assign(targetID, *component());
	return rootEntity->mInstanceProperties.at(idx);
}

std::string PropertyPath::getComponentInstancePath() const
{
	if (mObjectPath.size() < 3)
		return {};

	// First object must be Scene
	assert(mDocument != nullptr);
	auto leadObject = mDocument->getObject(mObjectPath[0].mID);
	if (!leadObject || !leadObject->get_type().is_derived_from<nap::Scene>())
		return {};

	// Second Object must be RootEntity
	auto secondObject = mDocument->getObject(mObjectPath[1].mID);
	if (!secondObject || !secondObject->get_type().is_derived_from<nap::Entity>())
		return {};

	// Last object must be Component
	auto trailObject = getObject();
	if (!trailObject || !trailObject->get_type().is_derived_from<nap::Component>())
		return {};

	std::vector<std::string> newPath(mObjectPath.begin() + 2, mObjectPath.begin() + mObjectPath.size());
	return "./" + nap::utility::joinString(newPath, "/");
}

nap::RootEntity* PropertyPath::getRootEntity() const
{
	if (mObjectPath.size() < 2)
		return nullptr;

	assert(mDocument != nullptr);
	auto scene = rtti_cast<nap::Scene>(mDocument->getObject(mObjectPath[0].mID));
	if (!scene)
		return nullptr;

	auto entity = rtti_cast<nap::Entity>(mDocument->getObject(mObjectPath[1].mID));
	if (!entity)
		return nullptr;

	auto nameIdx = mObjectPath[1].mIndex; int idx = 0;
	for (auto& rootEntity : scene->mEntities)
	{
		if (idx == nameIdx && rootEntity.mEntity.get() == entity)
			return &rootEntity;

		if (rootEntity.mEntity.get() == entity)
			++idx;
	}
	return nullptr;
}


nap::Component* PropertyPath::component() const
{
	return rtti_cast<nap::Component>(getObject());
}


nap::TargetAttribute* PropertyPath::targetAttribute() const
{
	auto instProps = instanceProps();
	if (!instProps)
		return nullptr;

	auto pathstr = propPathStr();
	for (auto& attr : instProps->mTargetAttributes)
	{
		if (attr.mPath == pathstr)
			return &attr;
	}
	return nullptr;
}


nap::TargetAttribute& PropertyPath::getOrCreateTargetAttribute()
{
	assert(isInstanceProperty());

	auto targetAttr = targetAttribute();
	if (targetAttr)
		return *targetAttr;

	// didn't exist, create
	auto& instProps = getOrCreateInstanceProps();

	auto pathstr = propPathStr();
	for (auto& attr : instProps.mTargetAttributes)
	{
		if (attr.mPath == pathstr)
			return attr;
	}

	auto idx = instProps.mTargetAttributes.size();
	instProps.mTargetAttributes.emplace_back();
	instProps.mTargetAttributes.at(idx).mPath = pathstr;
	return instProps.mTargetAttributes.at(idx);
}


rttr::variant PropertyPath::getValue() const
{
	if (isInstanceProperty() && isOverridden())
	{
		auto target_attr = targetAttribute();
		if (target_attr)
		{
			return getInstancePropertyValue(*target_attr->mValue.get());
		}
	}
	return resolve().getValue();
}


bool PropertyPath::setValue(rttr::variant value)
{
	// Regular property
	auto resolved_path = resolve();
	if (!isInstanceProperty())
	{
		return resolved_path.setValue(value);
	}

	//////////////////////////////////////////////////////////////////////////
	// TODO: Drastically improve handling of instance properties!!!
	// The current implementation is shaky at best. It works, but that's about it. What to do?
	// Properly handle all types of nap::InstancePropertyValue! (no more exceptions)
	// Properly implement callbacks, they're too scattered and hard to trace!
	// Strengthen and simplify the entire model!
	//////////////////////////////////////////////////////////////////////////

	// Instance property: If the value is the same, remove property or bail
	if (resolved_path.getValue() == value)
	{
		auto target_attr = targetAttribute();
		if (target_attr != nullptr)
		{
			rttr::variant val = target_attr->mValue.get();
			removeInstanceValue(target_attr, val);
		}
		return true;
	}

	// Get instance property (as target). Create one if it doesn't exist.
	// If it does exist: discard instance property value if the provided value is the same as the original
	auto target_attr = targetAttribute();
	if (target_attr == nullptr)
	{
		target_attr = &getOrCreateTargetAttribute();
		target_attr->mValue = createInstanceProperty(getType(), *getDocument());
	}

	// Set instance property value
	rttr::variant val = target_attr->mValue.get();
	return setInstancePropertyValue(val, value);
}


void PropertyPath::removeInstanceValue(const nap::TargetAttribute* targetAttr, rttr::variant& val) const
{
	// remove from target attributes list
	auto instProps = this->instanceProps();
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

	// Notify listeners
	auto component = instProps->mTargetComponent.get();
	mDocument->objectChanged(component);
	for (auto scene : mDocument->getObjects<nap::Scene>())
	{
		mDocument->objectChanged(scene);
	}
}


Object* PropertyPath::getPointee() const
{
	if (!isPointer())
		return nullptr;

	auto value = getValue();
	auto type = value.get_type();
	auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : type;
	return wrappedType != type ?
		value.extract_wrapped_value().get_value<nap::rtti::Object*>() :
		value.get_value<nap::rtti::Object*>();
}


void PropertyPath::setPointee(Object* pointee)
{
	// TODO: This is a hack to find ComponentPtr/ObjectPtr/EntityPtr method
	// Someone just needs to add an 'assign' method in the wrong place and it will break.
	// Also, ObjectPtr's assign method starts with uppercase A
	if (rttr::method assignMethod = nap::rtti::findMethodRecursive(getType(), "assign"))
	{
		// Assign the new value to the pointer (note that we're modifying a copy)
		auto targetVal = getValue();
		assert(mDocument != nullptr);
		auto path = mDocument->relativeObjectPath(*getObject(), *pointee);
		assignMethod.invoke(targetVal, path, *pointee);

		// Apply the modified value back to the source property
		setValue(targetVal);
	}
	else
	{
		setValue(pointee);
	}
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

	VariantArray array_view = array.create_array_view();
	return array_view.is_dynamic();
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
	if (hasProperty())
		return objectPathStr() + "@" + propPathStr();
	return objectPathStr();
}

bool PropertyPath::isInstanceProperty() const
{
	return hasProperty() && getRootEntity();
}

PropertyPath PropertyPath::getChild(const std::string& name) const
{
	assert(mDocument != nullptr);
	return {objectPathStr(), propPathStr() + "/" + name, *mDocument};
}

nap::rtti::Object* PropertyPath::getObject() const
{
	if (mObjectPath.empty())
		return nullptr;

	assert(mDocument != nullptr);
	return mDocument->getObject(mObjectPath.back().mID);
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
	if (!hasProperty())
		return false;
	return targetAttribute();
}

void PropertyPath::removeOverride()
{
	auto at = targetAttribute();
	if (!at)
		return;
	rttr::variant val = at->mValue.get();
	removeInstanceValue(at, val);
}

bool PropertyPath::hasOverriddenChildren() const
{
	if (isOverridden())
		return true;

	for (auto child : getChildren(IterFlag::Resursive))
		if (child.isOverridden())
			return true;

	return false;
}

bool PropertyPath::hasProperty() const
{
	return !mPropertyPath.empty();
}

bool PropertyPath::isValid() const
{
	// Path must be associated with a document and object
	if (mDocument == nullptr || getObject() == nullptr)
		return false;

	// If it's a property, resolve and validate
	return hasProperty() ? resolve().isValid() : true;
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
	if (isArray())
		return getArrayElementType().is_pointer();
	return getWrappedType().is_pointer();
}

bool PropertyPath::isEmbeddedPointer() const
{
	if (!isPointer())
		return false;

	return nap::rtti::hasFlag(getProperty(), EPropertyMetaData::Embedded);
}

bool PropertyPath::isNonEmbeddedPointer() const
{
	if (!isPointer())
		return false;

	return !nap::rtti::hasFlag(getProperty(), EPropertyMetaData::Embedded);
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

int PropertyPath::getRealChildEntityIndex() const
{
	auto parent = getParent();
	assert(parent.isValid());
	assert(parent.getType().is_derived_from<nap::Entity>());
	auto parentEntity = rtti_cast<nap::Entity>(parent.getObject());
	assert(parentEntity);

	int foundIDs = 0;
	int instanceIndex = getInstanceChildEntityIndex();
	for (int i = 0, len = static_cast<int>(parentEntity->mChildren.size()); i < len; i++)
	{
		auto currChild = parentEntity->mChildren[i];
		if (getObject()->mID == currChild->mID)
		{
			if (foundIDs == instanceIndex)
				return i;
			foundIDs++;
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

	auto pointee = getPointee();

	// prune here if there is no pointer value
	if (!pointee)
		return;

	for (auto childprop : pointee->get_type().get_properties())
	{
		auto childValue = childprop.get_value(pointee);
		std::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		PPath op;
		op.emplace_back(pointee->mID);

		PPath p;
		p.emplace_back(name);

		// This path points to the pointee
		assert(mDocument != nullptr);
		PropertyPath childPath(op, p, *mDocument);

		if (!visitor(childPath))
			return;
	}

}


std::string PropertyPath::objectPathStr() const
{
	std::vector<std::string> elements;
	for (const auto& elm : mObjectPath)
		elements.emplace_back(elm);
	return "/" + nap::utility::joinString(elements, "/");
}


std::string PropertyPath::propPathStr() const
{
	std::vector<std::string> elements;
	for (const auto& elm : mPropertyPath)
		elements.emplace_back(elm);
	return nap::utility::joinString(elements, "/");
}


void PropertyPath::updateObjectName(const std::string& oldName, const std::string& newName)
{
	for (auto& nameIdx : mObjectPath)
	{
		if (nameIdx.mID == oldName)
			nameIdx.mID = newName;
	}
}


void PropertyPath::invalidate()
{
	mDocument = nullptr;
}
