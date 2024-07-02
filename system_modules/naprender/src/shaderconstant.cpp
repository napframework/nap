/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "shaderconstant.h"

// NAP includes
#include <rtti/typeinfo.h>

RTTI_BEGIN_CLASS(nap::ShaderConstant, "Assigns a value to a specialization constant in a shader")
	RTTI_PROPERTY("Name", &nap::ShaderConstant::mName, nap::rtti::EPropertyMetaData::Default,	"Constant name in shader")
	RTTI_PROPERTY("Value", &nap::ShaderConstant::mValue, nap::rtti::EPropertyMetaData::Default, "Constant value")
RTTI_END_CLASS
