#include "propertypath.h"
#include "naputils.h"
#include "typeconversion.h"

#include <rtti/linkresolver.h>
#include <rtti/defaultlinkresolver.h>
#include <appcontext.h>
#include <stack>

using namespace nap::rtti;

napkin::PropertyPath::PropertyPath(Object& obj)
		: mObject(&obj)
{
}

napkin::PropertyPath::PropertyPath(Object& obj, const std::string& instPath)
		: mObject(&obj), mCompInstancePath(instPath)
{
	ResolvedPath resolvedPath;
	auto propPath = Path::fromString(instPath);
	if (propPath.resolve(mObject, resolvedPath))
		mPath = propPath;
	else
		mCompInstancePath = instPath;
}

napkin::PropertyPath::PropertyPath(nap::RootEntity& rootEntity, nap::rtti::Object& obj)
		: mRootEntity(&rootEntity), mObject(&obj)
{
}

napkin::PropertyPath::PropertyPath(nap::RootEntity& rootEntity, Object& obj, const std::string& instPath)
		: mRootEntity(&rootEntity), mObject(&obj), mCompInstancePath(instPath)
{
}

napkin::PropertyPath::PropertyPath(nap::RootEntity* rootEntity, nap::rtti::Object& obj, const std::string& instPath,
								   const nap::rtti::Path& propPath)
		: mRootEntity(rootEntity), mObject(&obj), mCompInstancePath(instPath), mPath(propPath)
{
}

napkin::PropertyPath::PropertyPath(const PropertyPath& parentPath, rttr::property prop)
		: mRootEntity(parentPath.mRootEntity),
		  mObject(parentPath.mObject),
		  mCompInstancePath(parentPath.mCompInstancePath)
{
	nap::rtti::Path path;
	path.pushAttribute(prop.get_name().data());
	mPath = path;
}

napkin::PropertyPath::PropertyPath(Object& obj, const Path& path)
		: mObject(&obj), mPath(path)
{
}


napkin::PropertyPath::PropertyPath(nap::rtti::Object& obj, rttr::property prop)
		: mObject(&obj)
{
	nap::rtti::Path path;
	path.pushAttribute(prop.get_name().data());
	mPath = path;
}

const std::string napkin::PropertyPath::getName() const
{
	if (hasProperty())
		return getProperty().get_name().data();
	return mObject->mID;
}

nap::ComponentInstanceProperties* napkin::PropertyPath::instanceProps() const
{
	if (!isInstanceProperty())
		return nullptr;

	auto pathstr = mPath.toString();

	// find instanceproperties
	if (mRootEntity->mInstanceProperties.empty())
		return nullptr;

	for (nap::ComponentInstanceProperties& instProp : mRootEntity->mInstanceProperties)
	{
		if (instProp.mTargetComponent.getInstancePath() == mCompInstancePath)
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
	auto idx = mRootEntity->mInstanceProperties.size();
	mRootEntity->mInstanceProperties.emplace_back();

	std::string targetID = componentInstancePath();

	mRootEntity->mInstanceProperties.at(idx).mTargetComponent.assign(targetID, *component());
	return mRootEntity->mInstanceProperties.at(idx);
}

std::string napkin::PropertyPath::componentInstancePath() const
{
	return mCompInstancePath;
}

nap::Component* napkin::PropertyPath::component() const
{
	return dynamic_cast<nap::Component*>(mObject);
}

nap::TargetAttribute* napkin::PropertyPath::targetAttribute() const
{
	auto pathstr = mPath.toString();

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

	auto pathstr = mPath.toString();
	for (auto& attr : instProps.mTargetAttributes)
	{
		if (attr.mPath == pathstr)
			return attr;
	}

	auto idx = instProps.mTargetAttributes.size();
	instProps.mTargetAttributes.emplace_back();
	instProps.mTargetAttributes.at(idx).mPath = mPath.toString();
	return instProps.mTargetAttributes.at(idx);
}

void napkin::PropertyPath::removeTargetAttribute()
{
	auto pathstr = mPath.toString();

	auto instProps = instanceProps();
	assert(instProps);

	// remove attribute
	auto& attrs = instProps->mTargetAttributes;
	attrs.erase(std::remove_if(attrs.begin(), attrs.end(),
							   [&pathstr](const nap::TargetAttribute& at)
							   {
								   return at.mPath == pathstr;
							   }),
				attrs.end());

	if (!attrs.empty())
		return; // there are overridden properties left, stop here

	// no more overrides, remove instanceproperties for this component
	auto& props = mRootEntity->mInstanceProperties;

	props.erase(std::remove_if(props.begin(), props.end(),
							   [this](const nap::ComponentInstanceProperties& instProp)
							   {
								   return instProp.mTargetComponent.getInstancePath() == mCompInstancePath;
							   }),
				props.end());
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
					auto& rootInstProps = mRootEntity->mInstanceProperties;
					rootInstProps.erase(std::remove_if(rootInstProps.begin(), rootInstProps.end(), flt), rootInstProps.end());

					AppContext::get().getDocument()->objectChanged(component);
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
		auto path = mPath;
		path.popBack();
		if (path.length() > 0)
			return {*mObject, path};
		return {*mObject};
	}

	if (!mCompInstancePath.empty())
	{
		auto slashIndex = mCompInstancePath.find_last_of('/');
		if (slashIndex > 0)
		{
			auto parentPath = mCompInstancePath.substr(0, slashIndex);
			auto parentID = parentPath.substr(parentPath.find_last_of('/')+1);

			auto idx = parentID.find(':');
			if (idx > 0)
				parentID = parentID.substr(0, idx);

			auto obj = AppContext::get().getDocument()->getObject(parentID);
			assert(obj);

			return {*obj, parentPath};
		}
	}
	return {};
}

rttr::property napkin::PropertyPath::getProperty() const
{
	return resolve().getProperty();
}

rttr::type napkin::PropertyPath::getType() const
{
	if (!mObject)
		return rttr::type::empty();

	if (!hasProperty())
		return mObject->get_type();

	Variant value = resolve().getValue();
	return value.get_type();
}

ResolvedPath napkin::PropertyPath::resolve() const
{
	ResolvedPath resolvedPath;
	mPath.resolve(mObject, resolvedPath);
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
	auto child_path = getPath();
	child_path.pushArrayElement(index);
	return {*mObject, child_path};
}


std::string napkin::PropertyPath::toString() const
{
	if (isInstanceProperty())
	{
		if (mPath.length())
			return componentInstancePath() + "/" + mPath.toString();
		return componentInstancePath();
	}

	auto objectPath = !mCompInstancePath.empty() ? mCompInstancePath : mObject->mID;

	// Is Property?
	auto propPathStr = mPath.toString();
	if (!propPathStr.empty())
		return objectPath + "@" + propPathStr;

	return objectPath;
}

napkin::PropertyPath napkin::PropertyPath::getChild(const std::string& name) const
{
	nap::rtti::Path child_path = getPath();
	child_path.pushAttribute(name);
	return {*mObject, child_path};
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
	return mPath.length() > 0;
}

bool napkin::PropertyPath::isValid() const
{
	// A valid path must always point to an object
	if (mObject == nullptr)
		return false;

	if (!hasProperty())
	{
		// points to object, not a property
		if (!mCompInstancePath.empty())
		{
			// Points to a component, check if the trailing elements is this object's ID
			auto elements = nap::utility::splitString(mCompInstancePath, '/');
			auto lastElement = elements[elements.size() - 1];
			return mObject->mID == lastElement;
		}
		return true;
	}

	auto resolvedPath = resolve();
	return resolvedPath.isValid();

}

bool napkin::PropertyPath::operator==(const napkin::PropertyPath& other) const
{
	if (getProperty() != other.getProperty())
		return false;
	if (&getObject() != &other.getObject())
		return false;

	return true;
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
		std::string path = doc->relativeObjectPath(*mObject, *pointee);

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
	if (!mObject)
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
	if (!mObject)
		return;

	for (rttr::property prop : mObject->get_type().get_properties())
	{
		PropertyPath path(*this, prop);
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
	// Take the previously set instance path and extract the index from the string
	auto path = mCompInstancePath;
	auto basename = nap::utility::splitString(mCompInstancePath, '/').back();
	auto parts = nap::utility::splitString(basename, ':');
	assert(parts.size() == 2);
	auto idxstring = parts.back();
	std::string::size_type sz;
	return std::stoi(idxstring, &sz);
}

int napkin::PropertyPath::getRealChildEntityIndex() const
{
	auto parent = getParent();
	assert(parent.isValid());
	assert(parent.getType().is_derived_from<nap::Entity>());
	auto parentEntity = dynamic_cast<nap::Entity*>(&parent.getObject());
	assert(parentEntity);

	int instanceIndex = getInstanceChildEntityIndex();

	int foundIDs = 0;
	for (int i=0, len = static_cast<int>(parentEntity->mChildren.size()); i < len; i++)
	{
		auto currChild = parentEntity->mChildren[i];
		if (mObject->mID == currChild->mID)
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
		nap::rtti::Path path = getPath();
		path.pushArrayElement(i);
		PropertyPath childPath(mRootEntity, *mObject, mCompInstancePath, path);

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
		auto path = mPath;
		path.pushAttribute(childProp.get_name().data());

		PropertyPath childPath(mRootEntity, *mObject, mCompInstancePath, path);

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

		nap::rtti::Path path;
		path.pushAttribute(name);

		// This path points to the pointee
		PropertyPath childPath(mRootEntity, *pointee, mCompInstancePath, path);

		if (!visitor(childPath))
			return;
	}

}
