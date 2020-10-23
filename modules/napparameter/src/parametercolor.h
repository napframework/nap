/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <parametersimple.h>
#include <color.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Color Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterRGBColorFloat	= ParameterSimple<RGBColorFloat>;
	using ParameterRGBAColorFloat	= ParameterSimple<RGBAColorFloat>;
	using ParameterRGBColor8		= ParameterSimple<RGBColor8>;
	using ParameterRGBAColor8		= ParameterSimple<RGBAColor8>;
}
