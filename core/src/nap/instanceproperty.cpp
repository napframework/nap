#include "instanceproperty.h"
#include "rtti/rttipath.h"

RTTI_BEGIN_CLASS(nap::ComponentInstanceProperties)
	RTTI_PROPERTY("TargetComponent",	&nap::ComponentInstanceProperties::mTargetComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TargetAttributes",	&nap::ComponentInstanceProperties::mTargetAttributes,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::TargetAttribute)
	RTTI_PROPERTY("Path", &nap::TargetAttribute::mPath, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Value", &nap::TargetAttribute::mValue, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::InstancePropertyValue)
RTTI_END_CLASS

RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::PointerInstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::ComponentPtrInstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::BoolInstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::CharInstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Int8InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Int16InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Int32InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Int64InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::UInt8InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::UInt16InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::UInt32InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::UInt64InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::FloatInstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::DoubleInstancePropertyValue)

namespace nap
{

	bool PointerInstancePropertyValue::setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const
	{
 		rtti::TypeInfo target_type = resolvedTargetPath.getType();
 		if (!errorState.check(target_type.is_derived_from(RTTI_OF(ObjectPtrBase)), "Target pointer is not an ObjectPtr"))
 			return false;

		rtti::TypeInfo actual_type = target_type.is_wrapper() ? target_type.get_wrapped_type() : target_type;
		if (!errorState.check(mValue->get_type().is_derived_from(actual_type), "Target is of the wrong type (found '%s', expected '%s')", mValue->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
			return false;

		return errorState.check(resolvedTargetPath.setValue(mValue), "Failed to set pointer to target %s", mValue->mID.c_str());
	}

	bool ComponentPtrInstancePropertyValue::setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const
	{
		rtti::TypeInfo target_type = resolvedTargetPath.getType();
		if (!errorState.check(target_type.is_derived_from(RTTI_OF(ComponentPtrBase)), "Target pointer is not a ComponentPtr"))
			return false;

		rtti::TypeInfo actual_type = target_type.is_wrapper() ? target_type.get_wrapped_type() : target_type;
		if (!errorState.check(mValue->get_type().is_derived_from(actual_type), "Target is of the wrong type (found '%s', expected '%s')", mValue->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
			return false;

		return errorState.check(resolvedTargetPath.setValue(mValue), "Failed to set pointer to target %s", mValue->mID.c_str());
	}

	bool TargetAttribute::apply(rtti::RTTIObject& target, utility::ErrorState& errorState) const
	{
		const rtti::RTTIPath rtti_path = rtti::RTTIPath::fromString(mPath);

		rtti::ResolvedRTTIPath resolved_path;
		if (!errorState.check(rtti_path.resolve(&target, resolved_path), "Path %s to instance property for component %s could not be resolved", mPath.c_str(), target.mID.c_str()))
			return false;

		return errorState.check(mValue->setValue(resolved_path, errorState), "Instance property for component %s and path %s could not be applied", target.mID.c_str(), mPath.c_str());
	}
}
