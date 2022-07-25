/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/group.h>

// Local Includes
#include "parameter.h"

namespace nap
{
	namespace group
	{
		namespace parameter
		{
			constexpr const char* members  = "Parameters";		///< Parameter Group members property name
			constexpr const char* children = "Groups";			///< Parameter Group children property name

		}
	}

	// Parameter group type definition
	using ParameterGroup = Group<Parameter>;

	// For backwards compatibility reasons, override the default 'Members' and 'Children' property names
	// of the 'nap::ParameterGroup' to the property names introduced before the arrival of the generic nap::Group<T>.
	template<>
	nap::Group<Parameter>::Group() :
		IGroup(RTTI_OF(Parameter), group::parameter::members, group::parameter::children)	{ }
}
