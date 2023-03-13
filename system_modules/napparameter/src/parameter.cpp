/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <parameter.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Parameter)
	RTTI_PROPERTY("Name",		&nap::Parameter::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{ }
