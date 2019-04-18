#include "propertypath.h"
#include "naputils.h"
#include "typeconversion.h"

#include <rtti/linkresolver.h>
#include <rtti/defaultlinkresolver.h>
#include <appcontext.h>
#include <stack>
#include <cctype>

using namespace nap::rtti;


napkin::NameIndex::NameIndex(const std::string& nameIndex)
{
	nameAndIndex(nameIndex, mID, mIndex);
}

std::string napkin::NameIndex::toString() const
{
	if (mIndex < 0)
		return mID;
	return mID + ":" + std::to_string(mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

napkin::PropertyPath::PropertyPath(Object& obj)
{
	mObjectPath.emplace_back(obj.mID);
}

napkin::PropertyPath::PropertyPath(const std::string& abspath)
{
	auto pathParts = nap::utility::splitString(abspath, '@');
	for (auto pathElm : nap::utility::splitString(pathParts[0], '/'))
		if (!pathElm.empty())
			mObjectPath.emplace_back(pathElm);

	if (pathParts.size() > 1)
		for (auto propElm : nap::utility::splitString(pathParts[1], '/'))
			mPropertyPath.emplace_back(propElm);

}

napkin::PropertyPath::PropertyPath(const std::string& abspath, const std::string& proppath)
{
	for (const auto& pathElm : nap::utility::splitString(abspath, '/'))
		if (!pathElm.empty())
			mObjectPath.emplace_back(pathElm);

	for (const auto& propElm : nap::utility::splitString(proppath, '/'))
		mPropertyPath.emplace_back(propElm);
}


napkin::PropertyPath::PropertyPath(const napkin::PPath& abspath)
		: mObjectPath(abspath)
{
}

napkin::PropertyPath::PropertyPath(const napkin::PPath& absPath, const napkin::PPath& propPath)
		: mObjectPath(absPath), mPropertyPath(propPath)
{
}


napkin::PropertyPath::PropertyPath(Object& obj, const Path& path)
{
	auto id = obj.mID;
	mObjectPath.emplace_back(NameIndex(id));
	mPropertyPath.emplace_back(path.toString());
}


napkin::PropertyPath::PropertyPath(nap::rtti::Object& obj, rttr::property prop)
{
	mObjectPath.emplace_back(obj.mID);
	mPropertyPath.emplace_back(std::string(prop.get_name().data()));
}

const std::string napkin::PropertyPath::getName() const
{
	if (hasProperty())
		return getProperty().get_name().data();
	return getObject()->mID;
}

nap::ComponentInstanceProperties* napkin::PropertyPath::instanceProps() const
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
		if (napkin::isComponentInstancePathEqual(*rootEntity,
												 *instProp.mTargetComponent.get(),
												 instProp.mTargetComponent.getInstancePath(),
												 compInstPath))
			return &instProp;
	}
	return nullptr;
}

nap::ComponentInstanceProperties& napkin::PropertyPath::getOrCreateInstanceProps()
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

std::string napkin::PropertyPath::getComponentInstancePath() const
{
	if (mObjectPath.size() < 3)
		return {};

	auto doc = document();

	// First object must be Scene
	auto leadObject = doc->getObject(mObjectPath[0].mID);
	if (!leadObject || !leadObject->get_type().is_derived_from<nap::Scene>())
		return {};

	// Second Object must be RootEntity
	auto secondObject = doc->getObject(mObjectPath[1].mID);
	if (!secondObject || !secondObject->get_type().is_derived_from<nap::Entity>())
		return {};

	// Last object must be Component
	auto trailObject = getObject();
	if (!trailObject || !trailObject->get_type().is_derived_from<nap::Component>())
		return {};

	std::vector<std::string> newPath(mObjectPath.begin() + 2, mObjectPath.begin() + mObjectPath.size());
	return "./" + nap::utility::joinString(newPath, "/");
}

nap::RootEntity* napkin::PropertyPath::getRootEntity() const
{
	if (mObjectPath.size() < 2)
		return nullptr;

	auto doc = document();

	auto scene = dynamic_cast<nap::Scene*>(doc->getObject(mObjectPath[0].mID));
	if (!scene)
		return nullptr;

	auto entity = dynamic_cast<nap::Entity*>(doc->getObject(mObjectPath[1].mID));
	if (!entity)
		return nullptr;

	auto nameIdx = mObjectPath[1].mIndex;

	int idx = 0;
	for (auto& rootEntity : scene->mEntities)
	{
		if (idx == nameIdx && rootEntity.mEntity.get() == entity)
			return &rootEntity;
		if (rootEntity.mEntity.get() == entity)
			++idx;
	}
	return nullptr;
}

nap::Component* napkin::PropertyPath::component() const
{
	return dynamic_cast<nap::Component*>(getObject());
}

nap::TargetAttribute* napkin::PropertyPath::targetAttribute() const
{
	auto pathstr = propPathStr();

	auto instProps = instanceProps();
	if (!instProps)
		return nullptr;

	for (auto& attr : instProps->mTargetAttributes)
	{
		if (attr.mPath == pathstr)
			return &attr;
	}

	return nullptr;
}

nap::TargetAttribute& napkin::PropertyPath::getOrCreateTargetAttribute()
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


rttr::variant napkin::PropertyPath::getValue() const
{
	auto targetAttr = targetAttribute();
	if (targetAttr)
	{
		if (getType() == rttr::type::get<float>())
			return dynamic_cast<nap::TypedInstancePropertyValue<float>*>(targetAttr->mValue.get())->mValue;
	}
	return resolve().getValue();
}

void napkin::PropertyPath::setValue(rttr::variant value)
{
	auto resolved = resolve();

	if (isInstanceProperty())
	{
		auto targetAttr = targetAttribute();
		if (targetAttr)
		{
			rttr::variant val = targetAttr->mValue.get();
			if (resolve().getValue() == value)
			{
				// instance value is the same as the original, remove instancepropertyvalue

				// remove from targetattributes list
				auto instProps = instanceProps();
				auto& attrs = instProps->mTargetAttributes;
				auto filter = [&](const nap::TargetAttribute& attr) { return &attr == targetAttr; };
				attrs.erase(std::remove_if(attrs.begin(), attrs.end(), filter), attrs.end());

				// remove attributes list if necessary
				if (attrs.empty())
				{
					auto component = instProps->mTargetComponent.get();
					auto flt = [&](const nap::ComponentInstanceProperties& instProp) { return &instProp == instProps; };
					auto& rootInstProps = getRootEntity()->mInstanceProperties;
					rootInstProps.erase(std::remove_if(rootInstProps.begin(), rootInstProps.end(), flt),
										rootInstProps.end());

					document()->objectChanged(component);
				}

				// Remove from object list
				napkin::removeInstancePropertyValue(val, getType());
			}
			else
			{
				napkin::setInstancePropertyValue(val, getType(), value);
			}
			return;
		}
		else
		{
			targetAttr = &getOrCreateTargetAttribute();
			targetAttr->mValue = napkin::createInstancePropertyValue(getType(), value);
		}
		return;
	}

	bool success = resolved.setValue(value);
	assert(success);
}

napkin::PropertyPath napkin::PropertyPath::getParent() const
{
	if (hasProperty())
	{
		if (mPropertyPath.size() > 1)
			return {mObjectPath, PPath(mPropertyPath.begin(), mObjectPath.begin() + mObjectPath.size() - 1)};

		if (mPropertyPath.size() == 1)
			return {mObjectPath};
	}

	if (mObjectPath.size() >= 2)
		return {PPath(mObjectPath.begin(), mObjectPath.begin() + mObjectPath.size() - 1)};

	return {};
}

rttr::property napkin::PropertyPath::getProperty() const
{
	return resolve().getProperty();
}

rttr::type napkin::PropertyPath::getType() const
{
	if (!getObject())
		return rttr::type::empty();

	if (!hasProperty())
		return getObject()->get_type();

	Variant value = resolve().getValue();
	return value.get_type();
}

ResolvedPath napkin::PropertyPath::resolve() const
{
	auto path = Path::fromString(propPathStr());
	ResolvedPath resolvedPath;
	path.resolve(getObject(), resolvedPath);
	return resolvedPath;
}


rttr::type napkin::PropertyPath::getArrayElementType() const
{
	ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_valid());
	if (!array.is_array())
		return rttr::type::empty();

	VariantArray array_view = array.create_array_view();
	auto elmtype = array_view.get_rank_type(1);
	//auto elmtype = array_view.get_rank_type(array_view.get_rank());
	return elmtype.is_wrapper() ? elmtype.get_wrapped_type() : elmtype;
}

size_t napkin::PropertyPath::getArrayLength() const
{
	ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_valid());
	assert(array.is_array());

	VariantArray array_view = array.create_array_view();
	assert(array_view.is_dynamic());
	assert(array_view.is_valid());

	return array_view.get_size();
}

napkin::PropertyPath napkin::PropertyPath::getArrayElement(size_t index) const
{
	if (!isArray())
		return PropertyPath();

	PPath p = mPropertyPath;
	p.emplace_back(std::to_string(index));
	return {mObjectPath, p};
}


std::string napkin::PropertyPath::toString() const
{
	if (hasProperty())
		return objectPathStr() + "@" + propPathStr();
	return objectPathStr();
}

bool napkin::PropertyPath::isInstanceProperty() const
{
	return hasProperty() && getRootEntity();
}

napkin::PropertyPath napkin::PropertyPath::getChild(const std::string& name) const
{
	return {objectPathStr(), propPathStr() + "/" + name};
}

nap::rtti::Object* napkin::PropertyPath::getObject() const
{
	if (mObjectPath.empty())
		return nullptr;

	return AppContext::get().getDocument()->getObject(mObjectPath.back().mID);
}

Path napkin::PropertyPath::getPath() const
{
	return Path::fromString(propPathStr());
}

rttr::type napkin::PropertyPath::getWrappedType() const
{
	const auto& type = getType();
	return type.is_wrapper() ? type.get_wrapped_type() : type;
}

bool napkin::PropertyPath::isOverridden() const
{
	if (!hasProperty())
		return false;
	return targetAttribute();
}

bool napkin::PropertyPath::hasOverriddenChildren() const
{
	if (isOverridden())
		return true;

	for (auto child : getChildren(IterFlag::Resursive))
		if (child.isOverridden())
			return true;

	return false;
}

bool napkin::PropertyPath::hasProperty() const
{
	return !mPropertyPath.empty();
}

bool napkin::PropertyPath::isValid() const
{
	// A valid path must always point to an object
	auto obj = getObject();
	if (!obj)
		return false;

	if (!hasProperty())
		return true;

	return resolve().isValid();
}

bool napkin::PropertyPath::operator==(const napkin::PropertyPath& other) const
{
	return objectPathStr() == other.objectPathStr() && propPathStr() == other.propPathStr();
}

bool napkin::PropertyPath::isArray() const
{
	return getType().is_array();
}

bool napkin::PropertyPath::isPointer() const
{
	if (isArray())
		return getArrayElementType().is_pointer();
	return getWrappedType().is_pointer();
}

bool napkin::PropertyPath::isEmbeddedPointer() const
{
	if (!isPointer())
		return false;

	return nap::rtti::hasFlag(getProperty(), EPropertyMetaData::Embedded);
}

bool napkin::PropertyPath::isNonEmbeddedPointer() const
{
	if (!isPointer())
		return false;

	return !nap::rtti::hasFlag(getProperty(), EPropertyMetaData::Embedded);
}


bool napkin::PropertyPath::isEnum() const
{
	return getWrappedType().is_enumeration();
}

Object* napkin::PropertyPath::getPointee() const
{
	if (!isPointer())
		return nullptr;

	ResolvedPath resolvedPath = resolve();
	auto value = resolvedPath.getValue();
	auto value_type = value.get_type();
	auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;

	if (wrapped_type != value_type)
		return value.extract_wrapped_value().get_value<nap::rtti::Object*>();
	else
		return value.get_value<nap::rtti::Object*>();
}

void napkin::PropertyPath::setPointee(Object* pointee)
{
	nap::rtti::ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());

	// TODO: This is a hack to find ComponentPtr/ObjectPtr/EntityPtr method
	// Someone just needs to add an 'assign' method in the wrong place and it will break.
	// Also, ObjectPtr's assign method starts with uppercase A
	rttr::method assign_method = nap::rtti::findMethodRecursive(resolved_path.getType(), "assign");
	if (assign_method.is_valid())
	{
		// Assign the new value to the pointer (note that we're modifying a copy)
		auto target_value = resolved_path.getValue();

		auto doc = AppContext::get().getDocument(); // TODO: This needs to go, but we need it to get a relative path.
		std::string path = doc->relativeObjectPath(*getObject(), *pointee);

		assign_method.invoke(target_value, path, *pointee);

		// Apply the modified value back to the source property
		bool value_set = resolved_path.setValue(target_value);
		assert(value_set);
	}
	else
	{
		bool value_set = resolved_path.setValue(pointee);
		if (pointee)
			assert(value_set);
	}
}

void napkin::PropertyPath::iterateChildren(std::function<bool(const napkin::PropertyPath&)> visitor, int flags) const
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

std::vector<napkin::PropertyPath> napkin::PropertyPath::getChildren(int flags) const
{
	std::vector<napkin::PropertyPath> children;

	iterateChildren([&children](const auto& path)
					{
						children.emplace_back(path);
						return true;
					}, flags);

	return children;
}

void napkin::PropertyPath::iterateProperties(napkin::PropertyVisitor visitor, int flags) const
{
	if (!getObject())
		return;

	for (rttr::property prop : getObject()->get_type().get_properties())
	{
		PPath propPath;
		propPath.emplace_back(std::string(prop.get_name().data()));
		PropertyPath path(mObjectPath, propPath);

		if (!visitor(path))
			return;

		if (flags & IterFlag::Resursive)
			path.iterateChildren(visitor, flags);
	}
}


std::vector<napkin::PropertyPath> napkin::PropertyPath::getProperties(int flags) const
{
	std::vector<napkin::PropertyPath> props;

	iterateProperties([&props](const auto& path)
					  {
						  props.emplace_back(path);
						  return true;
					  }, flags);

	return props;
}

int napkin::PropertyPath::getInstanceChildEntityIndex() const
{
	if (mObjectPath.empty())
		return -1;

	return mObjectPath.back().mIndex;
}

int napkin::PropertyPath::getRealChildEntityIndex() const
{
	auto parent = getParent();
	assert(parent.isValid());
	assert(parent.getType().is_derived_from<nap::Entity>());
	auto parentEntity = dynamic_cast<nap::Entity*>(parent.getObject());
	assert(parentEntity);

	int instanceIndex = getInstanceChildEntityIndex();

	int foundIDs = 0;
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

void napkin::PropertyPath::iterateArrayElements(napkin::PropertyVisitor visitor, int flags) const
{
	auto value = getValue();
	auto array = value.create_array_view();

	for (int i = 0; i < array.get_size(); i++)
	{
		PPath p = mPropertyPath;
		p.emplace_back(std::to_string(i));
		PropertyPath childPath(mObjectPath, p);

		if (!visitor(childPath))
			return;

		if (flags & IterFlag::Resursive)
			childPath.iterateChildren(visitor, flags);
	}
}

void napkin::PropertyPath::iterateChildrenProperties(napkin::PropertyVisitor visitor, int flags) const
{
	for (auto childProp : getType().get_properties())
	{
		PPath p = mPropertyPath;
		p.emplace_back(std::string(childProp.get_name().data()));
		PropertyPath childPath(mObjectPath, p);

		if (!visitor(childPath))
			return;

		if (flags & IterFlag::Resursive)
			childPath.iterateChildren(visitor, flags);
	}
}

void napkin::PropertyPath::iteratePointerProperties(napkin::PropertyVisitor visitor, int flags) const
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
		PropertyPath childPath(op, p);

		if (!visitor(childPath))
			return;
	}

}

napkin::Document* napkin::PropertyPath::document() const
{
	return AppContext::get().getDocument();
}

std::string napkin::PropertyPath::objectPathStr() const
{
	std::vector<std::string> elements;
	for (const auto& elm : mObjectPath)
		elements.emplace_back(elm);
	return "/" + nap::utility::joinString(elements, "/");
}

std::string napkin::PropertyPath::propPathStr() const
{
	std::vector<std::string> elements;
	for (const auto& elm : mPropertyPath)
		elements.emplace_back(elm);
	return nap::utility::joinString(elements, "/");
}

