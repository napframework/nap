#pragma once

#include "psblendmodes.h"

#include <glm/glm.hpp>
#include <nap.h>


namespace nap
{

	template <typename T, typename F>
	class BlendOperator : public Operator
	{

	public:
		BlendOperator() : Operator() {}

        virtual T doBlend() = 0;
		void doBlend(T& outValue) { outValue = blendFunc(target.getValue(), blend.getValue()); }

		InputPullPlug<T> target = {this, "target"};
		InputPullPlug<T> blend = {this, "blend"};
		OutputPullPlug<T> result = {this, "blend", &BlendOperator::doBlend};

	protected:
        T blendFunc;
//		virtual T blendFunc(const T& target, const T& blend) = 0;
	};
    // TODO: implement blend operators
}