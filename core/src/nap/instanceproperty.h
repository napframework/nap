#pragma once

// Local Includes
#include "objectptr.h"
#include "componentptr.h"
#include "rtti/rttiobject.h"

// External Includes

namespace nap
{	
	#define RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(InstancePropertyValueType)											\
		RTTI_BEGIN_CLASS(InstancePropertyValueType)																	\
			RTTI_PROPERTY("Value", &InstancePropertyValueType::mValue, nap::rtti::EPropertyMetaData::Required)		\
		RTTI_END_CLASS

	namespace rtti
	{
		class ResolvedRTTIPath;
	}

	namespace utility
	{
		class ErrorState;
	}

	class Component;

	class InstancePropertyValue : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const = 0;
	};

	class PointerInstancePropertyValue : public InstancePropertyValue
	{
		RTTI_ENABLE(InstancePropertyValue)

	public:
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const override;

	public:
		ObjectPtr<RTTIObject>		mValue;
	};

	class ComponentPtrInstancePropertyValue : public InstancePropertyValue
	{
		RTTI_ENABLE(InstancePropertyValue)
	public:
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const override;

	public:
		ComponentPtr<Component>		mValue;
	};

	template<class T>
	class TypedInstancePropertyValue : public InstancePropertyValue
	{
		RTTI_ENABLE(InstancePropertyValue)

	public:
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const override
		{
			rtti::TypeInfo target_type = resolvedTargetPath.getType();
			if (!errorState.check(target_type == rtti::TypeInfo::get<T>(), "Target value does not match instance property type %s", rtti::TypeInfo::get<T>().get_name().data()))
				return false;

			return errorState.check(resolvedTargetPath.setValue(mValue), "Failed to set integer value");
		}

	public:
		T mValue;
	};

	class TargetAttribute
	{
	public:
		bool apply(rtti::RTTIObject& target, utility::ErrorState& errorState) const;

		std::string							mPath;
		ObjectPtr<InstancePropertyValue>	mValue;
	};

	class ComponentInstanceProperties
	{
	public:
		using TargetAttributeList = std::vector<TargetAttribute>;
		ComponentPtr<Component>		mTargetComponent;
		TargetAttributeList			mTargetAttributes;
	};

	using BoolInstancePropertyValue = TypedInstancePropertyValue<bool>;
	using CharInstancePropertyValue = TypedInstancePropertyValue<char>;
	using Int8InstancePropertyValue = TypedInstancePropertyValue<int8_t>;
	using Int16InstancePropertyValue = TypedInstancePropertyValue<int16_t>;
	using Int32InstancePropertyValue = TypedInstancePropertyValue<int32_t>;
	using Int64InstancePropertyValue = TypedInstancePropertyValue<int64_t>;
	using UInt8InstancePropertyValue = TypedInstancePropertyValue<uint8_t>;
	using UInt16InstancePropertyValue = TypedInstancePropertyValue<uint16_t>;
	using UInt32InstancePropertyValue = TypedInstancePropertyValue<uint32_t>;
	using UInt64InstancePropertyValue = TypedInstancePropertyValue<uint64_t>;
	using FloatInstancePropertyValue = TypedInstancePropertyValue<float>;
	using DoubleInstancePropertyValue = TypedInstancePropertyValue<double>;
}
