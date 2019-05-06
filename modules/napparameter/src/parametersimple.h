#pragma once

// Local Includes
#include "parameter.h"

// External Includes
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * Parameter that simply wraps a value without any further metadata
	 */
	template<typename T>
	class ParameterSimple : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:

		/**
		 * Set the value of this enum from another parameter
		 *
		 * @param value The parameter to set the value from
		 */
		virtual void setValue(const Parameter& value) override;

		/**
		 * Set the value of this parameter. Will raise the valueChanged signal if the value actually changes.
		 *
		 * @param value The value to set
		 */
		void setValue(const T& value);

	public:
		T			mValue;				///< Property: 'Value' the value of this parameter
		Signal<T>	valueChanged;		///< Signal that's raised when the value of this parameter changes
	};


	//////////////////////////////////////////////////////////////////////////
	// Simple Parameter Declarations
	//////////////////////////////////////////////////////////////////////////

	using ParameterBool = ParameterSimple<bool>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ParameterSimple<T>::setValue(const Parameter& value)
	{
		const ParameterSimple<T>* derived_type = rtti_cast<const ParameterSimple<T>>(&value);
		assert(derived_type != nullptr);
		setValue(derived_type->mValue);
	}

	template<typename T>
	void nap::ParameterSimple<T>::setValue(const T& value)
	{
		if (value != mValue)
		{
			mValue = value;
			valueChanged(mValue);
		}
	}
}

/**
 * Helper macro that can be used to define the RTTI for a simple parameter type
 */
#define DEFINE_SIMPLE_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,		nap::rtti::EPropertyMetaData::Default)					\
	RTTI_END_CLASS