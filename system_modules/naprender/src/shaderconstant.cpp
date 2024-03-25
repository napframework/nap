/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "shaderconstant.h"

// NAP includes
#include <rtti/typeinfo.h>

RTTI_BEGIN_CLASS(nap::ShaderConstant)
	RTTI_PROPERTY("Name", &nap::ShaderConstant::mName, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Value", &nap::ShaderConstant::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
