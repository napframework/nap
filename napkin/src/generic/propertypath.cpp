#include "propertypath.h"

#include <rtti/rttiobject.h>
#include <rtti/linkresolver.h>
#include <rtti/defaultlinkresolver.h>
#include <appcontext.h>
#include <nap/logger.h>

using namespace nap::rtti;

napkin::PropertyPath::PropertyPath(RTTIObject& obj, const RTTIPath& path)
		: mObject(&obj), mPath(path)
{
}

napkin::PropertyPath::PropertyPath(RTTIObject& obj, const std::string& path)
		: mObject(&obj), mPath(RTTIPath())
{
	mPath.pushAttribute(path);
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
	return getValue().get_type();
}

ResolvedRTTIPath napkin::PropertyPath::resolve() const
{
	ResolvedRTTIPath resolvedPath;
	mPath.resolve(mObject, resolvedPath);
	return resolvedPath;
}


rttr::type napkin::PropertyPath::getArrayElementType()
{
	auto arrayView = getArrayView();
	return arrayView.get_rank_type(arrayView.get_rank());
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
	nap::rtti::RTTIPath child_path = path();
	child_path.pushAttribute(name);
	return {*mObject, child_path};
}

rttr::type napkin::PropertyPath::getWrappedType() const
{
	auto value = getValue();
	return value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();
}

bool napkin::PropertyPath::isValid() const
{
	if (mObject == nullptr)
		return false;
	auto resolvedPath = resolve();
	if (!resolvedPath.isValid())
		return false;

	return true;
}



