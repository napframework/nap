#include "propertypath.h"
#include "naputils.h"

#include <rtti/object.h>
#include <rtti/linkresolver.h>
#include <rtti/defaultlinkresolver.h>
#include <appcontext.h>
#include <nap/logger.h>
#include <stack>
#include <QtDebug>

using namespace nap::rtti;

napkin::PropertyPath::PropertyPath(const napkin::PropertyPath& other)
		: mObject(&other.getObject()), mPath(other.getPath())
{
}

napkin::PropertyPath::PropertyPath(nap::rtti::Object& obj)
		: mObject(&obj)
{
}

napkin::PropertyPath::PropertyPath(nap::RootEntity& rootEntity, nap::rtti::Object& obj)
		: mRootEntity(&rootEntity), mObject(&obj)
{
}

napkin::PropertyPath::PropertyPath(Object& obj, const Path& path)
		: mObject(&obj), mPath(path)
{
}

napkin::PropertyPath::PropertyPath(Object& obj, const std::string& path)
		: mObject(&obj), mPath(Path::fromString(path))
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

rttr::variant napkin::PropertyPath::getValue() const
{
	rttr::property prop = getProperty();
	return prop.get_value(mObject);
}

void napkin::PropertyPath::setValue(rttr::variant value)
{
	bool success = getProperty().set_value(mObject, value);
	assert(success);
}

napkin::PropertyPath napkin::PropertyPath::getParent() const
{
	auto path = mPath;
	path.popBack();
	return {*mObject, path};
}

rttr::property napkin::PropertyPath::getProperty() const
{
	return resolve().getProperty();
}

rttr::type napkin::PropertyPath::getType() const
{
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
	return nap::utility::stringFormat("%s@%s", mObject->mID.c_str(), mPath.toString().c_str());
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

bool napkin::PropertyPath::isValid() const
{
	if (mObject == nullptr)
		return false;

	if (!hasProperty())
		return true; // points to object, not a property

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

	if (!hasProperty())
	{
		for (auto p : getProperties(*mObject, flags))
		{
			assert(p.isValid());
			p.iterateChildren(visitor, flags);
		}
		return;
	}

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


void napkin::PropertyPath::iterateProperties(nap::rtti::Object& obj,
											 std::function<bool(const napkin::PropertyPath&)> visitor, int flags)
{
	for (rttr::property prop : obj.get_type().get_properties())
	{
		PropertyPath path(obj, prop);
		if (!visitor(path))
			return;

		if (flags & IterFlag::Resursive)
			path.iterateChildren(visitor, flags);
	}
}

std::vector<napkin::PropertyPath> napkin::PropertyPath::getProperties(nap::rtti::Object& obj, int flags)
{
	std::vector<napkin::PropertyPath> props;

	iterateProperties(obj, [&props](const auto& path)
	{
		props.emplace_back(path);
		return true;
	}, flags);

	return props;
}

void napkin::PropertyPath::iterateArrayElements(napkin::PropertyVisitor visitor, int flags) const
{
	auto value = getValue();
	auto array = value.create_array_view();

	for (int i = 0; i < array.get_size(); i++)
	{
		nap::rtti::Path path = getPath();
		path.pushArrayElement(i);
		PropertyPath childPath(*mObject, path);

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

		PropertyPath childPath(*mObject, path);

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
		PropertyPath childPath(*pointee, path);

		if (!visitor(childPath))
			return;
	}

}







