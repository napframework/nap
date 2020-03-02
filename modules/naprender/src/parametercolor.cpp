// Local Includes
#include "parametercolor.h"

DEFINE_SIMPLE_PARAMETER(nap::ParameterRGBColorFloat)
DEFINE_SIMPLE_PARAMETER(nap::ParameterRGBAColorFloat)
DEFINE_SIMPLE_PARAMETER(nap::ParameterRGBColor8)
DEFINE_SIMPLE_PARAMETER(nap::ParameterRGBAColor8)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterRGBAFloatBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterRGBFloatBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterRGBA8Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterRGB8Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

namespace nap
{
	template<>
	void nap::ParameterBlender<ParameterRGBAColorFloat, RGBAColorFloat>::onBlend(float value)
	{
		glm::vec4 lval = math::lerp<glm::vec4>(mSourceValue.toVec4(), getTarget<ParameterRGBAColorFloat>().mValue.toVec4(), value);
		RGBAColorFloat nval(lval.x, lval.y, lval.z, lval.w);
		getParameter<ParameterRGBAColorFloat>().setValue(nval);
	}


	template<>
	void nap::ParameterBlender<ParameterRGBColorFloat, RGBColorFloat>::onBlend(float value)
	{
		glm::vec3 lval = math::lerp<glm::vec3>(mSourceValue.toVec3(), getTarget<ParameterRGBColorFloat>().mValue.toVec3(), value);
		RGBColorFloat nval(lval.x, lval.y, lval.z);
		getParameter<ParameterRGBColorFloat>().setValue(nval);
	}

	template<>
	void nap::ParameterBlender<ParameterRGBAColor8, RGBAColor8>::onBlend(float value)
	{
		glm::vec4 lval = math::lerp<glm::vec4>(mSourceValue.toVec4(), getTarget<ParameterRGBAColor8>().mValue.toVec4(), value);
		RGBAColor8 nval(lval.x, lval.y, lval.z, lval.w);
		getParameter<ParameterRGBAColor8>().setValue(nval);
	}


	template<>
	void nap::ParameterBlender<ParameterRGBColor8, RGBColor8>::onBlend(float value)
	{
		glm::vec3 lval = math::lerp<glm::vec3>(mSourceValue.toVec3(), getTarget<ParameterRGBColor8>().mValue.toVec3(), value);
		RGBColor8 nval(lval.x, lval.y, lval.z);
		getParameter<ParameterRGBColor8>().setValue(nval);
	}
}
