/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "parametergroup.h"

// For backwards compatibility reasons, override the the default 'Members' and 'Children' property names
// of the 'nap::ParameterGroup' to the property names introduced before the arrival of the generic nap::Group<T>.
RTTI_BEGIN_CLASS(nap::ParameterGroup)
	RTTI_PROPERTY(nap::group::parameter::members,	&nap::ParameterGroup::mMembers,		nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)
	RTTI_PROPERTY(nap::group::parameter::children,	&nap::ParameterGroup::mChildren,	nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)
RTTI_END_CLASS

namespace nap
{
	template<> 
	nap::Group<Parameter>::Group() : IGroup(RTTI_OF(Parameter),
		group::parameter::members, group::parameter::children) { }
}
