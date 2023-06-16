/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "parameterblender.h"
#include <mathutils.h>
#include <nap/logger.h>

RTTI_DEFINE_BASE(nap::BaseParameterBlender)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterFloatBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterDoubleBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterVec2Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterVec3Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterVec4Blender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterBoolBlender)
	RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterQuatBlender)
        RTTI_CONSTRUCTOR(nap::Parameter&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Contains all the available parameter blenders
	 */
	static std::unordered_map<rtti::TypeInfo, rtti::TypeInfo> sBlendMap
	{
		{ RTTI_OF(ParameterFloat),			RTTI_OF(ParameterFloatBlender)		},
		{ RTTI_OF(ParameterDouble),			RTTI_OF(ParameterDoubleBlender)		},
		{ RTTI_OF(ParameterVec2),			RTTI_OF(ParameterVec2Blender)		},
		{ RTTI_OF(ParameterVec3),			RTTI_OF(ParameterVec3Blender)		},
		{ RTTI_OF(ParameterVec4),           RTTI_OF(ParameterVec4Blender)		},
		{ RTTI_OF(ParameterRGBAColorFloat),	RTTI_OF(ParameterRGBAFloatBlender)	},
		{ RTTI_OF(ParameterRGBColorFloat),	RTTI_OF(ParameterRGBFloatBlender)	},
		{ RTTI_OF(ParameterRGBAColor8),		RTTI_OF(ParameterRGBA8Blender)		},
		{ RTTI_OF(ParameterRGBColor8),		RTTI_OF(ParameterRGB8Blender)		},
		{ RTTI_OF(ParameterBool),			RTTI_OF(ParameterBoolBlender)		},
        { RTTI_OF(ParameterQuat),           RTTI_OF(ParameterQuatBlender)       }
	};


	std::unique_ptr<BaseParameterBlender> NAPAPI getParameterBlender(Parameter& param)
	{
		// Find in map
		auto it = sBlendMap.find(param.get_type());
		if (it == sBlendMap.end())
			return nullptr;

		// Create blender
		nap::BaseParameterBlender* blender = it->second.create<BaseParameterBlender>({ param });
		return std::unique_ptr<BaseParameterBlender>(blender);
	}


	bool registerParameterBlender(rtti::TypeInfo inParameterType, rtti::TypeInfo inBlenderType)
	{
		if (!inParameterType.is_derived_from(RTTI_OF(nap::Parameter)))
		{
			nap::Logger::warn("Unable to register parameter blender, parameter %s: not derived from %s", inParameterType.get_name().to_string().c_str(),
				RTTI_OF(Parameter).get_name().to_string().c_str());
			return false;
		}

		if (!inBlenderType.is_derived_from(RTTI_OF(nap::BaseParameterBlender)))
		{
			nap::Logger::warn("Unable to register parameter blender, blender %s: not derived from %s", inBlenderType.get_name().to_string().c_str(),
				RTTI_OF(BaseParameterBlender).get_name().to_string().c_str());
			return false;
		}

		auto it = sBlendMap.emplace(std::make_pair(inParameterType, inBlenderType));
		if (!(it.second))
		{
			nap::Logger::warn("Unable to register parameter blender, parameter %s: duplicate entry",
				inParameterType.get_name().to_string().c_str());
			return false;
		}
		return true;
	}


	BaseParameterBlender::BaseParameterBlender(Parameter& parameter) : mParameter(&parameter)
	{ }


	void BaseParameterBlender::blend(float value)
	{
		assert(mTarget != nullptr);
		onBlend(math::clamp<float>(value, 0.0f, 1.0f));
	}


	void BaseParameterBlender::setTarget(const Parameter* target)
	{
		// Ensure target is derived from parameter
		if (!target->get_type().is_derived_from(mParameter->get_type()))
		{
			assert(false);
			return;
		}

		// Update target and call derived classes
		mTarget = target;
		onTargetSet();
	}


	void BaseParameterBlender::clearTarget()
	{
		mTarget = nullptr;
	}


	void BaseParameterBlender::sync()
	{
		onSync();
	}


	bool BaseParameterBlender::hasTarget() const
	{
		return mTarget != nullptr;
	}


	const nap::Parameter& BaseParameterBlender::getTarget() const
	{
		assert(mTarget != nullptr);
		return *mTarget;
	}


	const nap::Parameter& BaseParameterBlender::getParameter() const
	{
		assert(mParameter != nullptr);
		return *mParameter;
	}


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


	template<>
	void nap::ParameterBlender<ParameterBool, bool>::onBlend(float value)
	{
		bool nval = value < 1.0f - math::epsilon<float>() ? mSourceValue : getTarget<ParameterBool>().mValue;
		getParameter<ParameterBool>().setValue(nval);
	}


    template<>
    void nap::ParameterBlender<ParameterQuat, glm::quat>::onBlend(float value)
    {
        glm::quat q = glm::slerp(mSourceValue, getTarget<ParameterQuat>().mValue, value);
        getParameter<ParameterQuat>().setValue(q);
    }
}
