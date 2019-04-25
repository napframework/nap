#include "instanceproperty.h"
#include "rtti/path.h"

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

// RTTI for instance properties for POD types
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
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Vec2InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Vec3InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Vec3InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::IVec2InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::IVec3InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::QuatInstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::StringInstancePropertyValue)

namespace nap
{

	bool PointerInstancePropertyValue::setValue(rtti::ResolvedPath& resolvedTargetPath, utility::ErrorState& errorState) const
	{
		// Check if target attribute is of correct type
 		rtti::TypeInfo target_type = resolvedTargetPath.getType();
 		if (!errorState.check(target_type.is_derived_from(RTTI_OF(rtti::ObjectPtrBase)), "Target pointer is not an ObjectPtr"))
 			return false;

		// Check if the internal (wrapped) type is of the correct type
		rtti::TypeInfo actual_type = target_type.is_wrapper() ? target_type.get_wrapped_type() : target_type;
		if (!errorState.check(mValue->get_type().is_derived_from(actual_type), "Target is of the wrong type (found '%s', expected '%s')", mValue->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
			return false;

		return errorState.check(resolvedTargetPath.setValue(mValue), "Failed to set pointer to target %s", mValue->mID.c_str());
	}

	//////////////////////////////////////////////////////////////////////////

	bool ComponentPtrInstancePropertyValue::setValue(rtti::ResolvedPath& resolvedTargetPath, utility::ErrorState& errorState) const
	{
		// Check if target attribute is of correct type
		rtti::TypeInfo target_type = resolvedTargetPath.getType();
		if (!errorState.check(target_type.is_derived_from(RTTI_OF(ComponentPtrBase)), "Target pointer is not a ComponentPtr"))
			return false;

		// Check if the internal (wrapped) type is of the correct type
		rtti::TypeInfo actual_type = target_type.is_wrapper() ? target_type.get_wrapped_type() : target_type;
		if (!errorState.check(mValue->get_type().is_derived_from(actual_type), "Target is of the wrong type (found '%s', expected '%s')", mValue->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
			return false;

		// ResolvedRTTIPath cannot deal with this specific case. We are trying to set a ComponentPtr<Component> to a ComponentPtr<targetType>. Because these types are not the same,
		// it will convert the ComponentPtr by using their wrapped types, but in this specific case we will lose the Component instance path that was on the object (it will only deal 
		// with the internal wrapped type and copy that data only).
		// So instead, we make sure that the types are fully matched by first getting the ComponentPtr<targetType>, then assigning the path to it; setValue will then not decide to use convert 
		// and will assign the value directly, without losing information.
		rtti::Variant new_value = resolvedTargetPath.getValue();
		ComponentPtrBase& new_component_ptr = const_cast<ComponentPtrBase&>(new_value.get_value<ComponentPtrBase>());
		new_component_ptr.assign(mValue.getInstancePath(), *mValue.get());

		return errorState.check(resolvedTargetPath.setValue(new_value), "Failed to set pointer to target %s", mValue->mID.c_str());
	}

	//////////////////////////////////////////////////////////////////////////

	bool TargetAttribute::apply(rtti::Object& target, utility::ErrorState& errorState) const
	{
		const rtti::Path rtti_path = rtti::Path::fromString(mPath);

		rtti::ResolvedPath resolved_path;
		if (!errorState.check(rtti_path.resolve(&target, resolved_path), "Path %s to instance property for component %s could not be resolved", mPath.c_str(), target.mID.c_str()))
			return false;

		return errorState.check(mValue->setValue(resolved_path, errorState), "Instance property for component %s and path %s could not be applied", target.mID.c_str(), mPath.c_str());
	}
}
