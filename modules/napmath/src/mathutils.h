#pragma once

// External Includes
#include <glm/glm.hpp>
#include <limits>
#include <utility/dllexport.h>
#include <algorithm>

namespace nap
{
	namespace math
	{
		/**
		 * PI as defined by cmath
		 */
		double NAPAPI pi();

		/**
		* Maps @inValue to new range defined by parameters
		* @return interpolated value
		*/
		template<typename T>
		float fit(T value, T min, T max, T outMin, T outMax);

		/**
		 * Blend between @start and @end value based on percent
		 * @param start minumum value
		 * @param end maximum value
		 * @param percent the amount to blend between start and end, 0-1
		 */
		template<typename T>
		T lerp(const T& start, const T& end, float percent);

		/**
		 *	Clamps value between min and max
		 */
		template<typename T>
		T clamp(T value, T min, T max);

		/**
		 *Returns the minumum of the Left and Right values
		 */
		template<typename T>
		T min(T left, T right);

		/**
		 *	Returns the maximum of the Left and Right values
		 */
		template<typename T>
		T max(T left, T right);

		/**
		 *	Rounds down a value
		 */
		template<typename T>
		T floor(T value);

		/**
		 *	Rounds up a value
		 */
		template<typename T>
		T ceil(T value);

		/**
		 *	@return value raised to power of exponent
		 */
		template<typename T>
		T power(T value, T exp);

		/**
		 * @return epsilon of T
		 */
		template<typename T>
		T epsilon();

		/**
		 *	@return the maximum possible value of T
		 */
		template<typename T>
		T max();
		
		/**
		 *	@return the lowest possible value of T
		 */
		template<typename T>
		T min();

		/**
		 * @return the sign of @value, 1 for values > 0, -1 for values < 0, 0 for values of exactly 0
		 * @param value the value to get the sign for
		 */
		template<typename T>
		T sign(T value);

		/**
		 * @return a bell shaped curve based on value t (0-1)
		 * @param time value in the range 0-1 to get value for
		 * @param strength exponent of the bell curve
		 */
		template<typename T>
		T bell(T time, T strength);

		/**
		 * @return a random number in range
		 * @param min min random number
		 * @param max max random number
		 */
		int NAPAPI random(int min, int max);

		/**
		 * Interpolates a value over time to a @target using a dampening model
		 * @param currentValue the current blend value, often the @return value
		 * @param targetValue the value to blend to
		 * @param currentVelocity the current velocity used to blend to @target
		 * @param time in seconds between cooks
		 * @param smoothTime approximately the time it will take to reach the target. A smaller value will reach the target faster.
		 * @param maxSpeed allows you to clamp the maximum speed
		 * @return the blended current value
		 */
		float NAPAPI smoothDamp(float currentValue, float targetValue, float& currentVelocity, float deltaTime, float smoothTime, float maxSpeed = 1000.0f);

		/**
		* Smoothly interpolates a value over time to a @target using a dampening model
		* @param currentValue the current blend value, will be updated after calling
		* @param targetValue the value to blend to
		* @param currentVelocity the current velocity used to blend to @target
		* @param time in seconds between cooks
		* @param smoothTime approximately the time it will take to reach the target. A smaller value will reach the target faster.
		* @param maxSpeed allows you to clamp the maximum speed
		* @return the blended current value
		*/
		template<typename T>
		void smooth(T& currentValue, const T& targetValue, T& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

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
		T clamp(T value, T min, T max)
		{
			return glm::clamp<T>(value, min, max);
		}

		template<typename T>
		T min(T left, T right)
		{
			return std::min<T>(left, right);
		}

		template<typename T>
		T max(T left, T right)
		{
			return std::max<T>(left, right);
		}

		template<typename T>
		T floor(T value)
		{
			return glm::floor(value);
		}

		template<typename T>
		T ceil(T value)
		{
			return glm::ceil(value);
		}

		template<typename T>
		T epsilon()
		{
			return std::numeric_limits<T>::epsilon();
		}

		template<typename T>
		T max()
		{
			return std::numeric_limits<T>::max();
		}

		template<typename T>
		T min()
		{
			return std::numeric_limits<T>::min();
		}

		template<typename T>
		T sign(T value)
		{
			return static_cast<T>(value == 0 ? 0 : value > 0 ? 1 : -1);
		}

		template<typename T>
		T bell(T t, T strength)
		{
			return power<T>(4.0f, strength) * power<T>(t *(1.0f - t), strength);
		}

		//////////////////////////////////////////////////////////////////////////
		// Forward declarations of templated lerp functions
		//////////////////////////////////////////////////////////////////////////

		template<>
		NAPAPI float lerp<float>(const float& start, const float& end, float percent);

		template<>
		NAPAPI double lerp<double>(const double& start, const double& end, float percent);

		template<>
		NAPAPI glm::vec2 lerp<glm::vec2>(const glm::vec2& start, const glm::vec2& end, float percent);

		template<>
		NAPAPI glm::vec3 lerp<glm::vec3>(const glm::vec3& start, const glm::vec3& end, float percent);

		template<>
		NAPAPI glm::vec4 lerp<glm::vec4>(const glm::vec4& start, const glm::vec4& end, float percent);

		template<>
		NAPAPI float power<float>(float value, float exp);

		template<>
		NAPAPI double power<double>(double value, double exp);

		template<>
		NAPAPI int power<int>(int value, int exp);

		template<>
		NAPAPI void smooth(float& currentValue, const float& targetValue, float& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI void smooth(glm::vec2& currentValue, const glm::vec2& targetValue, glm::vec2& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI void smooth(glm::vec3& currentValue, const glm::vec3& targetValue, glm::vec3& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI void smooth(glm::vec4& currentValue, const glm::vec4& targetValue, glm::vec4& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);
	}
}
