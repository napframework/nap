#pragma once

// Local Includes
#include "componentptr.h"
#include <rtti/rttiobject.h>
#include <rtti/rttipath.h>
#include <nap/objectptr.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

// External Includes

namespace nap
{	
	// Helper macro to register RTTI for instance properties for POD types
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

	/**
	 * Base class for all typed instance property values. This represents the value that is applied on a certain property.
	 */
	class InstancePropertyValue : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:

		/**
		 * Applies an instance property value to the target path. The value itself should be stored on the derived typed class.
		 * @param resolvedTargetPath Path to the property to modify. The Path is constructed from both the ComponentPtr and the RTTIPath.
		 * @param errorState If function returns false, contains error information if an error occurs, like types that do not match.
		 * @return True on success, otherwise false.
		 */
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const = 0;
	};

	/**
	 * Instance property value for pointer type.
	 */
	class PointerInstancePropertyValue : public InstancePropertyValue
	{
		RTTI_ENABLE(InstancePropertyValue)

	public:
		/**
		 * Sets pointer value.
		 * @param resolvedTargetPath Path to the property to modify. The Path is constructed from both the ComponentPtr and the RTTIPath.
		 * @param errorState If function returns false, contains error information if an error occurs, like types that do not match.
		 * @return True on success, otherwise false.
		 */
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const override;

	public:
		ObjectPtr<RTTIObject>		mValue;		// Pointer override value
	};

	/**
	 * Instance property value a ComponentPtr type. The value contains a path as if it was present on the original attribute. So, any
	 * relative paths are relative to the original location of the property.
	 */
	class ComponentPtrInstancePropertyValue : public InstancePropertyValue
	{
		RTTI_ENABLE(InstancePropertyValue)
	public:

		/**
		 * Sets component pointer value.
		 * @param resolvedTargetPath Path to the property to modify. The Path is constructed from both the ComponentPtr and the RTTIPath.
		 * @param errorState If function returns false, contains error information if an error occurs, like types that do not match.
		 * @return True on success, otherwise false.
		 */
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const override;

	public:
		ComponentPtr<Component>		mValue;		// Component pointer override value
	};

	/**
	 * Template class for instance property value for POD-like types. A number of 'usings' are used to create specific types
	 * for int, float etc, so that RTTI can be created for all these types.
	 */
	template<class T>
	class TypedInstancePropertyValue : public InstancePropertyValue
	{
		RTTI_ENABLE(InstancePropertyValue)

	public:
		/**
		 * Sets value.
		 * @param resolvedTargetPath Path to the property to modify. The Path is constructed from both the ComponentPtr and the RTTIPath.
		 * @param errorState If function returns false, contains error information if an error occurs, like types that do not match.
		 * @return True on success, otherwise false.
		 */
		virtual bool setValue(rtti::ResolvedRTTIPath& resolvedTargetPath, utility::ErrorState& errorState) const override
		{
			rtti::TypeInfo target_type = resolvedTargetPath.getType();
			if (!errorState.check(target_type == rtti::TypeInfo::get<T>(), "Target value does not match instance property type %s", rtti::TypeInfo::get<T>().get_name().data()))
				return false;

			return errorState.check(resolvedTargetPath.setValue(mValue), "Failed to set integer value");
		}

	public:
		T	mValue;	// Override value
	};

	/**
	 * Represents both the path to an attribute as well as the value to override. Together with the target object that must be given 
	 * in apply(), this can be used to override a property.
	 */
	class TargetAttribute
	{
	public:
		/**
		 * Applies the stored override value @mValue on target @target and attribute path @mPath.
		 * @param target The object that holds the attribute.
		 * @param errorState If function returns false, contains error information if an error occurs.
		 * @return True on success, otherwise false.
		 */
		bool apply(rtti::RTTIObject& target, utility::ErrorState& errorState) const;

		std::string							mPath;			///< RTTI path to the property
		ObjectPtr<InstancePropertyValue>	mValue;			///< Value to override
	};

	/**
	 * Represents all the properties that are overridden van a single component.
	 */
	class ComponentInstanceProperties
	{
	public:
		using TargetAttributeList = std::vector<TargetAttribute>;
		ComponentPtr<Component>		mTargetComponent;		///< Component to override properties from
		TargetAttributeList			mTargetAttributes;		///< List of values that are overridden
	};

	/**
	 * Instance property types for all the POD types
	 */
	using BoolInstancePropertyValue		= TypedInstancePropertyValue<bool>;
	using CharInstancePropertyValue		= TypedInstancePropertyValue<char>;
	using Int8InstancePropertyValue		= TypedInstancePropertyValue<int8_t>;
	using Int16InstancePropertyValue	= TypedInstancePropertyValue<int16_t>;
	using Int32InstancePropertyValue	= TypedInstancePropertyValue<int32_t>;
	using Int64InstancePropertyValue	= TypedInstancePropertyValue<int64_t>;
	using UInt8InstancePropertyValue	= TypedInstancePropertyValue<uint8_t>;
	using UInt16InstancePropertyValue	= TypedInstancePropertyValue<uint16_t>;
	using UInt32InstancePropertyValue	= TypedInstancePropertyValue<uint32_t>;
	using UInt64InstancePropertyValue	= TypedInstancePropertyValue<uint64_t>;
	using FloatInstancePropertyValue	= TypedInstancePropertyValue<float>;
	using DoubleInstancePropertyValue	= TypedInstancePropertyValue<double>;
	using Vec2InstancePropertyValue		= TypedInstancePropertyValue<glm::vec2>;
	using Vec3InstancePropertyValue		= TypedInstancePropertyValue<glm::vec3>;
	using Vec4InstancePropertyValue		= TypedInstancePropertyValue<glm::vec4>;
	using IVec2InstancePropertyValue	= TypedInstancePropertyValue<glm::ivec2>;
	using IVec3InstancePropertyValue	= TypedInstancePropertyValue<glm::ivec3>;
	using QuatInstancePropertyValue		= TypedInstancePropertyValue<glm::quat>;
}