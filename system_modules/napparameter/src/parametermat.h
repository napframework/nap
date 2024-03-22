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
	 * A numeric matrix parameter can hold glm::mat3 and glm::mat4
	 * The values associated with the matrix are clamped to the specified min and max bounds.
	 */
	template<typename T>
	class ParameterMat : public Parameter
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
		void setValue(T value);

	public:
		T						mValue;										        ///< Property: 'Value' the value of this parameter
		Signal<T>				valueChanged;								        ///< Signal that's raised when the value changes
	};


	//////////////////////////////////////////////////////////////////////////
	// Matrix Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterMat3		= ParameterMat<glm::mat3>;
	using ParameterMat4		= ParameterMat<glm::mat4>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void ParameterMat<T>::setValue(const Parameter& value)
	{
		const ParameterMat<T>* derived_type = rtti_cast<const ParameterMat<T>>(&value);
		assert(derived_type != nullptr);
		setValue(derived_type->mValue);
	}

	template<typename T>
	void ParameterMat<T>::setValue(T value)
	{
		// Set the components for each element individually
		T oldValue = mValue;
		mValue = value;

		if (oldValue != mValue)
			valueChanged(mValue);
	}
}

/**
 * Helper macro that can be used to define the RTTI for a matrix parameter type
 */
#define DEFINE_MATRIX_PARAMETER(Type)																		\
	RTTI_BEGIN_CLASS(Type)																					\
		RTTI_PROPERTY("Value",		&Type::mValue,			nap::rtti::EPropertyMetaData::Default)			\
		RTTI_FUNCTION("setValue",	static_cast<void (Type::*)(decltype(Type::mValue))>(&Type::setValue))	\
	RTTI_END_CLASS
