/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "tweeneasing.h"

// External Includes
#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::ETweenEaseType)
	RTTI_ENUM_VALUE(nap::ETweenEaseType::LINEAR,			"Linear"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::CUBIC_IN,			"Cubic In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::CUBIC_INOUT,		"Cubic In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::CUBIC_OUT,			"Cubic Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::BACK_IN,			"Back In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::BACK_INOUT,		"Back InOut"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::BACK_OUT,			"Back Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::BOUNCE_IN,			"Bounce In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::BOUNCE_INOUT,		"Bounce In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::BOUNCE_OUT, 		"Bounce Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::CIRC_IN,			"Circ In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::CIRC_INOUT,		"Circ In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::CIRC_OUT, 			"Circ Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::ELASTIC_IN,		"Elastic In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::ELASTIC_INOUT,		"Elastic In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::ELASTIC_OUT, 		"Elastic Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::EXPO_IN,			"Expo In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::EXPO_INOUT,		"Expo In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::EXPO_OUT, 			"Expo Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUAD_IN,			"Quad In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUAD_INOUT,		"Quad In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUAD_OUT,			"Quad Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUART_IN,			"Quart In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUART_INOUT,		"Quart In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUART_OUT,			"Quart Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUINT_IN,			"Quint In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUINT_INOUT,		"Quint In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::QUINT_OUT,			"Quint Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::SINE_IN,			"Sine In"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::SINE_INOUT,		"Sine In Out"),
	RTTI_ENUM_VALUE(nap::ETweenEaseType::SINE_OUT,			"Sine Out")
RTTI_END_ENUM