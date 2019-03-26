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
	 * It should be specialized for an enum type and the specialization should be registered in RTTI.
	 * It is assumed that the enum being specialized for has also been registered in RTTI
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
		virtual void setValue(const Parameter& value) override
		{
			const ParameterEnum<T>* derived_type = rtti_cast<const ParameterEnum<T>>(&value);
			assert(derived_type != nullptr);

			setValue(derived_type->mValue);
		}

		/**
		 * Set the value of this enum. Will raise the valueChanged signal if the value actually changes.
		 *
		 * @param value The value to set
		 */
		void setValue(T value)
		{
			T oldValue = mValue;
			mValue = value;
			if (oldValue != mValue)
			{
				valueChanged(mValue);
			}
		}

	public:
		T			mValue;			///< Property: 'Value' the current value of the parameter
		Signal<T>	valueChanged;	///< Signal that's raised when the value of this parameter changes
	};
}
