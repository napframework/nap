#pragma once

// External Includes
#include <parametersimple.h>
#include <color.h>
#include <parameterblender.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Color Parameter Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterRGBColorFloat	= ParameterSimple<RGBColorFloat>;
	using ParameterRGBAColorFloat	= ParameterSimple<RGBAColorFloat>;
	using ParameterRGBColor8		= ParameterSimple<RGBColor8>;
	using ParameterRGBAColor8		= ParameterSimple<RGBAColor8>;


	//////////////////////////////////////////////////////////////////////////
	// Color Blender Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterRGBAFloatBlender = ParameterBlender<ParameterRGBAColorFloat, RGBAColorFloat>;
	using ParameterRGBFloatBlender	= ParameterBlender<ParameterRGBColorFloat, RGBColorFloat>;
	using ParameterRGBA8Blender		= ParameterBlender<ParameterRGBAColor8, RGBAColor8>;
	using ParameterRGB8Blender		= ParameterBlender<ParameterRGBColor8, RGBColor8>;


	//////////////////////////////////////////////////////////////////////////
	// Color Blender Template Specializations
	//////////////////////////////////////////////////////////////////////////

	template<>
	void nap::ParameterBlender<ParameterRGBAColorFloat, RGBAColorFloat>::onBlend(float value);

	template<>
	void nap::ParameterBlender<ParameterRGBColorFloat, RGBColorFloat>::onBlend(float value);

	template<>
	void nap::ParameterBlender<ParameterRGBAColor8, RGBAColor8>::onBlend(float value);

	template<>
	void nap::ParameterBlender<ParameterRGBColor8, RGBColor8>::onBlend(float value);
}
