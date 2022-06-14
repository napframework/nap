/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "group.h"

// nap::Resource run time class definition 
RTTI_BEGIN_CLASS(nap::Group)
	RTTI_PROPERTY("Resources", &nap::Group::mResources, nap::rtti::EPropertyMetaData::Embedded | nap::rtti::EPropertyMetaData::ReadOnly)
RTTI_END_CLASS

namespace nap
{ }
