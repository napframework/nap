#pragma once

// External Includes
#include <glm/glm.hpp>
#include <limits>
#include <utility/dllexport.h>

namespace nap
{
	namespace math
	{
		/**
		* Maps @inValue to new range defined by parameters
		* @return interpolated value
		*/
		template<typename T>
		float fit(T value, T min, T max, T outMin, T outMax);

		template<typename T>
		T lerp(T start, T end, float percent);

		template<typename T>
		T clamp(T value, T min, T max);

		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		float fit(T value, T min, T max, T outMin, T outMax)
		{
			T v = glm::clamp<T>(value, min, max);
			T m = max - min;
			m = (m == 0.0f) ? std::numeric_limits<T>::epsilon() : m;
			return (v - min) / (m) * (outMax - outMin) + outMin;
		}

		template<typename T>
		T lerp(T start, T end, float percent)
		{
			return glm::mix<T>(start, end, percent);
		}

		template<typename T>
		T clamp(T value, T min, T max)
		{
			return glm::clamp<T>(value, min, max);
		}
	}
}
