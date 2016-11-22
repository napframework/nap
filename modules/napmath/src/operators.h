#pragma once

#include "psblendmodes.h"

#include <glm/glm.hpp>
#include <nap.h>

#define NAP_DEFINE_BLEND_OPERATOR(NAME, FUNC, TYPE)                                               \
	template <typename TYPE>                                                                      \
	\
class NAME##BlendOperator : public Operator{\
protected : \
TYPE blend(const TYPE& target, const TYPE& blend) override{return nap::math::FUNC(target, blend); \
	}                                                                                             \
	\
}                                                                                          \
	;


namespace nap
{

	template <typename T>
	class BlendOperator : public Operator
	{

	public:
		BlendOperator() : Operator() {}


		void doBlend(T& outValue) { outValue = blendFunc(target.getValue(), blend.getValue()); }

		InputPullPlug<T> target = {this, "target"};
		InputPullPlug<T> blend = {this, "blend"};
		OutputPullPlug<T> result = {this, "blend", &BlendOperator::doBlend};

	protected:
		virtual T blendFunc(const T& target, const T& blend) = 0;
	};

	template <typename glm::vec2>
	class ScreenBlendOperator : public Operator
	{
	protected:
		glm::vec2 blend(const glm::vec2& target, const glm::vec2& blend) override
		{
			return nap::math::screen(target, blend);
		}
	};

	using BlendOperatorVec3 = BlendOperator<glm::vec3>;
}