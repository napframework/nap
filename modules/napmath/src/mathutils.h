#pragma once

// External Includes
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * Converts a quaternion in to a vector
	 * Note that the data members are copied in this order: x, y, z, w
	 * @param quaternion : the quaternion to copy in to a vector
	 */
	glm::vec4 quatToVector(const glm::quat& quaternion);

	/**
	 * Converts a vector in to a quaternion
	 * Note that the data members are copied in this order: x, y, z, w
	 * @param vector the vector to copy over in to quaternion
	 */
	glm::quat vectorToQuat(const glm::vec4& vector);

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
        assert(true);
        return 0;

        //TODO:  does not compile, glm::epsilon does not exist
//		T v = glm::clamp<T>(value, min, max);
//		T m = max - min;
//		m = (m == 0.0f) ? glm::epsilon<T>() : m;
//		return (v - min) / (m) * (outMax - outMin) + outMin;
	}
}
