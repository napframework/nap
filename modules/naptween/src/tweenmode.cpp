/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "tweenmode.h"

// External Includes
#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::ETweenMode)
	RTTI_ENUM_VALUE(nap::ETweenMode::NORMAL,	"Normal"),
	RTTI_ENUM_VALUE(nap::ETweenMode::LOOP,		"Loop"),
	RTTI_ENUM_VALUE(nap::ETweenMode::PING_PONG, "Ping Pong"),
	RTTI_ENUM_VALUE(nap::ETweenMode::REVERSE,	"Reverse")
RTTI_END_ENUM
