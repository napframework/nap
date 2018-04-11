#include "propertypath.h"
#include "naputils.h"

#include <rtti/object.h>
#include <rtti/linkresolver.h>
#include <rtti/defaultlinkresolver.h>
#include <appcontext.h>
#include <nap/logger.h>

using namespace nap::rtti;

napkin::PropertyPath::PropertyPath(const napkin::PropertyPath& other)
		: mObject(&other.getObject()), mPath(other.getPath())
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

rttr::variant napkin::PropertyPath::getValue() const
{
	return getProperty().get_value(mObject);
}

void napkin::PropertyPath::setValue(rttr::variant value)
{
	bool success = getProperty().set_value(mObject, value);
	assert(success);
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
	auto elmtype = array_view.get_rank_type(array_view.get_rank());
	return elmtype.is_wrapper() ? elmtype.get_wrapped_type() : elmtype;
}

size_t napkin::PropertyPath::getArrayLength()const
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

	if(wrapped_type != value_type)
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
		assert(value_set);
	}
}







