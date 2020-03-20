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
