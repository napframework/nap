#pragma once

// External Includes
#include <glm/glm.hpp>
#include <limits>

namespace nap
{
	/**
	 * Maps @inValue to new range defined by parameters
	 * @return interpolated value
	 */
	template<typename T>
	float fit(T value, T min, T max, T outMin, T outMax);


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	float fit(T value, T min, T max, T outMin, T outMax)
	{
        // TODO: does not compile, glm::epsilon does not exist - Stijn
		//
		// Why would this compile on my machine? glm::epsilon is defined
		// in glm::constants and resolves to std::numeric_limits<T>::epsilon
		// NOW we do use glm, and I assume all the same version of glm. If you
		// Get a compile error, lmk and I can fix it, or try to properly resolve 
		// it yourself. This assert(true); return 0; broke my entire render pipe and cost me
		// an afternoon to resolve because of a lazy assumption. Please be more careful - Coen
		T v = glm::clamp<T>(value, min, max);
		T m = max - min;
		m = (m == 0.0f) ? std::numeric_limits<T>::epsilon() : m;
		return (v - min) / (m) * (outMax - outMin) + outMin;
	}
}
