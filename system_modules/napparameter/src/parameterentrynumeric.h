/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <parameternumeric.h>

namespace nap
{
	/**
	 * A Parameter Entry wraps both a Numeric Parameter and default value so that a ResourcePtr to the Parameter is optional
	 */
	template<typename T>
	class ParameterNumericEntry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ResourcePtr<ParameterNumeric<T>> mParameter;
		T mDefault;

		/**
		 * @return true whether the current entry holds a valid reference to a Parameter.
		 */
		bool hasParameter() const				{ return (mParameter != nullptr); }

		/**
		 * Returns the parameter value if it is available, otherwise returns the default value.
		 * @return the value of this parameter entry
		 */
		T getValue() const						{ return hasParameter() ? mParameter->mValue : mDefault; }
	};

	//////////////////////////////////////////////////////////////////////////
	// Numeric Parameter Entry Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterEntryFloat	= ParameterNumericEntry<float>;
	using ParameterEntryInt		= ParameterNumericEntry<int>;
	using ParameterEntryByte	= ParameterNumericEntry<uint8_t>;
	using ParameterEntryDouble	= ParameterNumericEntry<double>;
	using ParameterEntryLong	= ParameterNumericEntry<int64_t>;
}

 /**
  * Helper macro that can be used to define the RTTI for a numeric parameter entry type
  */
#define DEFINE_NUMERIC_PARAMETER_ENTRY(Type)													\
	RTTI_BEGIN_CLASS(Type)																		\
		RTTI_PROPERTY("Parameter",	&Type::mParameter,	nap::rtti::EPropertyMetaData::Default)	\
		RTTI_PROPERTY("Default",	&Type::mDefault,	nap::rtti::EPropertyMetaData::Default)	\
	RTTI_END_CLASS
