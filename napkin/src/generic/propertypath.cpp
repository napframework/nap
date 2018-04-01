#include "propertypath.h"

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
		: mObject(&obj), mPath(Path())
{
	std::vector<std::string> split = nap::utility::splitString(path, '/');
	for (auto part : split)
		mPath.pushAttribute(part);
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
	return getProperty().get_type();
}

ResolvedPath napkin::PropertyPath::resolve() const
{
	ResolvedPath resolvedPath;
	mPath.resolve(mObject, resolvedPath);
	return resolvedPath;
}


rttr::type napkin::PropertyPath::getArrayElementType()
{
	ResolvedPath resolved_path = resolve();
	assert(resolved_path.isValid());

	Variant array = resolved_path.getValue();
	assert(array.is_valid());
	if (!array.is_array())
		return rttr::type::empty();

	VariantArray array_view = array.create_array_view();
	auto arrayView = getArrayView();
	return arrayView.get_rank_type(arrayView.get_rank());
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

rttr::variant_array_view napkin::PropertyPath::getArrayView()
{
	mResolvedPath = resolve();
	mVariant = mResolvedPath.getValue();
	mVariantArray = mVariant.create_array_view();
	return mVariantArray;
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

bool napkin::PropertyPath::isEnum() const
{
	if (!isValid())
		return false;
	return getType().is_enumeration();
}

bool napkin::PropertyPath::isPointer() const
{
	if (!isValid())
		return false;

	const auto& type = getType();
	const nap::rtti::TypeInfo wrapped_type = type.is_wrapper() ? type.get_wrapped_type() : type;
	// TODO: There must be a less convoluted way.
	// In the case of array elements, the type will be the array type, not the element type.
	// For now, grab the array's element type and use that.
	nap::rtti::TypeInfo wrapped_array_type = rttr::type::empty();
	if (type.is_array()) {
		nap::rtti::Variant value = getValue();
		nap::rtti::VariantArray array = value.create_array_view();
		nap::rtti::TypeInfo array_type = array.get_rank_type(array.get_rank());
		wrapped_array_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;
	}

	return wrapped_type.is_pointer() || wrapped_array_type.is_pointer();
}





