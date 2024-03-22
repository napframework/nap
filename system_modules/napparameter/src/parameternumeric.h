/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mathutils.h"

// External Includes
#include <parameter.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * A numeric parameter can hold any numeric type and provides a minimum & maximum value to ensure the value stays within range
	 */
	template<typename T>
	class ParameterNumeric : public Parameter
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
		virtual void setValue(T value);

		/**
		 * Sets the min/max range of this parameter to the specified values.
		 * If the current value is outside of the specified range, it will be clamped.
		 *
		 * @param minimum The minimum value for this parameter
		 * @param maximum The maximum value for this parameter
		 */
		void setRange(T minimum, T maximum);

	public:
		T			mValue;							///< Property: 'Value' the value of this parameter
		T			mMinimum = static_cast<T>(0);	///< Property: 'Minimum' the minimum value of this parameter
		T			mMaximum = static_cast<T>(1);	///< Property: 'Maximum' the maximum value of this parameter

		Signal<T>	valueChanged;					///< Signal that's raised when the value of this parameter changes
	};


	//////////////////////////////////////////////////////////////////////////
	// Numeric Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterFloat	= ParameterNumeric<float>;
	using ParameterInt		= ParameterNumeric<int>;
	using ParameterUInt		= ParameterNumeric<uint>;
	using ParameterByte		= ParameterNumeric<uint8_t>;
	using ParameterDouble	= ParameterNumeric<double>;
	using ParameterLong		= ParameterNumeric<int64_t>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ParameterNumeric<T>::setValue(const Parameter& value)
	{
		const ParameterNumeric<T>* derived_type = rtti_cast<const ParameterNumeric<T>>(&value);
		assert(derived_type != nullptr);

		// Set value from the parameter
		setValue(derived_type->mValue);
	}

	template<typename T>
	void ParameterNumeric<T>::setValue(T value)
	{
		T oldValue = mValue;
		mValue = math::clamp(value, mMinimum, mMaximum);
		if (oldValue != mValue)
		{
			valueChanged(mValue);
		}
	}

	template<typename T>
	void ParameterNumeric<T>::setRange(T minimum, T maximum)
	{
		mMinimum = minimum;
		mMaximum = maximum;
		setValue(mValue);
	}
}

/**
 * Helper macro that can be used to define the RTTI for a numeric parameter type
 */
#define DEFINE_NUMERIC_PARAMETER(Type)																		\
	RTTI_BEGIN_CLASS(Type)																					\
		RTTI_PROPERTY("Value",		&Type::mValue,			nap::rtti::EPropertyMetaData::Default)			\
		RTTI_PROPERTY("Minimum",	&Type::mMinimum,		nap::rtti::EPropertyMetaData::Default)			\
		RTTI_PROPERTY("Maximum",	&Type::mMaximum,		nap::rtti::EPropertyMetaData::Default)			\
		RTTI_FUNCTION("setValue",	static_cast<void (Type::*)(decltype(Type::mValue))>(&Type::setValue))	\
	RTTI_END_CLASS
