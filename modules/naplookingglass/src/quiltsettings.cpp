/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "quiltsettings.h"
#include <rtti/typeinfo.h>

RTTI_BEGIN_CLASS(nap::QuiltSettings)
	RTTI_PROPERTY("Width",		&nap::QuiltSettings::mWidth,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",		&nap::QuiltSettings::mHeight,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rows",		&nap::QuiltSettings::mRows,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Columns",	&nap::QuiltSettings::mColumns,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
