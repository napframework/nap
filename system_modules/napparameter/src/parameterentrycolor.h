/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // External Includes
#include <parametercolor.h>

namespace nap
{
	/**
	 * Parameter Color Entry
	 * Wraps parameter and default value so that a reference to the parameter is optional
	 */
	template<typename T>
	class ParameterColorEntry : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ResourcePtr<ParameterSimple<T>> mParameter;
		T mDefault;

		bool hasParameter() const { return (mParameter != nullptr); }
		const T& getValue() const { return hasParameter() ? mParameter->mValue : mDefault; }
	};

	//////////////////////////////////////////////////////////////////////////
	// Numeric Parameter Entry Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterEntryRGBColorFloat	= ParameterColorEntry<RGBColorFloat>;
	using ParameterEntryRGBAColorFloat	= ParameterColorEntry<RGBAColorFloat>;
	using ParameterEntryRGBColor8		= ParameterColorEntry<RGBColor8>;
	using ParameterEntryRGBAColor8		= ParameterColorEntry<RGBAColor8>;
}

/**
 * Helper macro that can be used to define the RTTI for a color parameter entry type
 */
#define DEFINE_COLOR_PARAMETER_ENTRY(Type)														\
	RTTI_BEGIN_CLASS(Type)																		\
		RTTI_PROPERTY("Parameter",	&Type::mParameter,	nap::rtti::EPropertyMetaData::Default)	\
		RTTI_PROPERTY("Default",	&Type::mDefault,	nap::rtti::EPropertyMetaData::Default)	\
	RTTI_END_CLASS
