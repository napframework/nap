#pragma once

// External Includes
#include <parameter.h>
#include <nap/signalslot.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Base class for enum parameters. This base is used to offer generic support for editing enums, without having to be aware of the specific enum types
	 */
	class NAPAPI ParameterEnumBase : public Parameter
	{
		RTTI_ENABLE(Parameter)

	public:
		ParameterEnumBase(rtti::TypeInfo enumType);

		/**
		 * Get the current value of this parameter as an int
		 *
		 * @return Current value of this parameter as an int
		 */
		virtual int getValue() const = 0;

		/**
		 * Set the value of this enum from an int
		 *
		 * @param value The value to set
		 */
		virtual void setValue(int value) = 0;

		/**
		 * Get the RTTI type of the enum represented by this parameter
		 */
		const rtti::TypeInfo& getEnumType() const { return mEnumType; }

	private:
		rtti::TypeInfo mEnumType;	///< The RTTI type of the enum represented by this parameter
	};

	/**
	 * ParameterEnum provides the concrete implementation of ParameterEnumBase. 
	 * Both the parameter type and the enum itself need to be registered in RTTI.
	 *
	 * Example:
	 *
	 * // SomeEnum.h
	 * enum class ESomeEnum
	 * {
	 *		SomeValue1,
	 *		SomeValue2
	 * }
	 *
	 * using SomeEnumParameter = ParameterEnum<ESomeEnum>;
	 *
	 * // SomeEnum.cpp
	 * // Regular RTTI definition for enum itself:
	 * RTTI_BEGIN_ENUM(ESomeEnum)
	 *		RTTI_ENUM_VALUE(ESomeEnum::SomeValue1,		"SomeValue1"),
	 *		RTTI_ENUM_VALUE(ESomeEnum::SomeValue2,		"SomeValue2")
	 * RTTI_END_ENUM
	 * 
	 * // RTTI definition for parameter:
	 * DEFINE_ENUM_PARAMETER(SomeEnumParameter)
	 */
	template<class T>
	class ParameterEnum : public ParameterEnumBase
	{
		RTTI_ENABLE(ParameterEnumBase)
	
	public:
		ParameterEnum() :
			ParameterEnumBase(RTTI_OF(T))
		{
		}

		/**
		 * Get the current value of this parameter as an int
		 *
		 * @return Current value of this parameter as an int
		 */
		virtual int getValue() const override { return (int)mValue; }

		/**
		 * Set the value of this enum from an int
		 *
		 * @param value The value to set
		 */
		virtual void setValue(int value) override { setValue((T)value); }

		/**
		 * Set the value of this enum from another parameter
		 *
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * Set the value of this enum. Will raise the valueChanged signal if the value actually changes.
		 *
		 * @param value The value to set
		 */
		void setValue(T value);

	public:
		T			mValue;			///< Property: 'Value' the current value of the parameter
		Signal<T>	valueChanged;	///< Signal that's raised when the value of this parameter changes
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<class T>
	void ParameterEnum<T>::setValue(const Parameter& value)
	{
		const ParameterEnum<T>* derived_type = rtti_cast<const ParameterEnum<T>>(&value);
		assert(derived_type != nullptr);

		setValue(derived_type->mValue);
	}

	template<class T>
	void ParameterEnum<T>::setValue(T value)
	{
		T oldValue = mValue;
		mValue = value;
		if (oldValue != mValue)
		{
			valueChanged(mValue);
		}
	}
}

#define DEFINE_ENUM_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)															\
		RTTI_PROPERTY("Value", &Type::mValue, nap::rtti::EPropertyMetaData::Default)					\
	RTTI_END_CLASS
