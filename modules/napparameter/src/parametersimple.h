/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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

		/**
		 * @return const reference to value
		 */
		const T& getValue() const;

	public:
		T			mValue;				///< Property: 'Value' the value of this parameter
		Signal<T>	valueChanged;		///< Signal that's raised when the value of this parameter changes
	};


	//////////////////////////////////////////////////////////////////////////
	// Simple Parameter Type Declarations
	//////////////////////////////////////////////////////////////////////////

	using ParameterBool = ParameterSimple<bool>;
	using ParameterString = ParameterSimple<std::string>;


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

	template<typename T>
	const T& nap::ParameterSimple<T>::getValue() const
	{
		return mValue;
	}
}

/**
 * Helper macro that can be used to define the RTTI for a simple parameter type
 */
#define DEFINE_SIMPLE_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,		nap::rtti::EPropertyMetaData::Default)					\
		RTTI_FUNCTION("setValue",	static_cast<void (Type::*)(const decltype(Type::mValue)&)>(&Type::setValue))\
	RTTI_END_CLASS
