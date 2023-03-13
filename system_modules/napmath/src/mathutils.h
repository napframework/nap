/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <glm/glm.hpp>
#include <nap/numeric.h>
#include <limits>
#include <utility/dllexport.h>
#include <algorithm>
#include <string>

namespace nap
{
	namespace math
	{
		inline constexpr double E			= 2.71828182845904523536;   // e
		inline constexpr double LOG2E		= 1.44269504088896340736;   // log2(e)
		inline constexpr double LOG10E		= 0.434294481903251827651;  // log10(e)
		inline constexpr double LN2			= 0.693147180559945309417;  // ln(2)
		inline constexpr double LN10		= 2.30258509299404568402;   // ln(10)
		inline constexpr double PI			= 3.14159265358979323846;   // pi
		inline constexpr double PIX2		= 6.28318530717958647692;	// pi*2
		inline constexpr double PI_2		= 1.57079632679489661923;   // pi/2
		inline constexpr double PI_4		= 0.785398163397448309616;  // pi/4
		inline constexpr double M1_PI		= 0.318309886183790671538;  // 1/pi
		inline constexpr double M2_PI		= 0.636619772367581343076;  // 2/pi
		inline constexpr double M2_SQRTPI	= 1.12837916709551257390;   // 2/sqrt(pi)
		inline constexpr double SQRT2		= 1.41421356237309504880;   // sqrt(2)
		inline constexpr double SQRT1_2		= 0.707106781186547524401;  // 1/sqrt(2)

		/**
		 * Maps a value from min/max to outMin/outMax.
		 * For example: fit<float>(10.0f, 0.0f, 20.0f, 100.0f, 200.0f) -> returns 150.0f.
		 * Input is clamped, ensuring output is always in outMin and outMax range.
		 * @param value the value to map
		 * @param min the min input value
		 * @param max the max input value
		 * @param outMin min output value
		 * @param outMax max output value
		 * @return mapped value
		 */
		template<typename T>
		float fit(T value, T min, T max, T outMin, T outMax);

		/**
		 * Blends a value between start and end value based on percentage (0-1).
		 * @param start begin value
		 * @param end end value
		 * @param percent the amount to blend between start and end, in between 0 and 1.
		 * @return interpolated value.
		 */
		template<typename T>
		T lerp(const T& start, const T& end, float percent);

		/**
		 * Clamp a value between min and max. 
		 * The value will never exceed max parameter or fall below min.
		 * @param value value to clamp.
		 * @param min lower limit of value.
		 * @param max upper limit of value. 
		 * @return value clamped to min and max.
		 */
		template<typename T>
		T clamp(T value, T min, T max);

		/**
		 * Returns the lowest of 2 values.
		 * @param left first value.
		 * @param right second value.
		 * @return the lowest value.
		 */
		template<typename T>
		T min(T left, T right);

		/**
		 * Returns the highest of 2 values.
		 * @param left first value.
		 * @param right second value.
		 * @return the highest value.
		 */
		template<typename T>
		T max(T left, T right);

		/**
		 * Rounds value downward, returning the largest integral value that is not greater than value.
		 * For example: 2.3 -> 2.0, -3.8 -> -4.0
		 * @param value value to floor.
		 * @return floored value of T
		 */
		template<typename T>
		T floor(T value);

		/**
		 * Rounds value upward, returning the smallest integral value that is not less than x.
		 * For example: 2.3 -> 3.0, -2.3 -> 2.0
		 * @param value value to ceil
		 * @return The smallest integral value that is not less than x (as a floating-point value).
		 */
		template<typename T>
		T ceil(T value);

		/**
		 * @param value parameter to raise.
		 * @param exp power exponent.
		 * @return Return base raised to the power exponent.
		 */
		template<typename T>
		T power(T value, T exp);

		/**
		 * Smoothly interpolates, using a Hermite polynomial, between 0 and 1
		 * @param value value to interpolate
		 * @param edge0 returns 0 if input is less than this value
		 * @param edge1 returns 1 if input is higher than this value
		 */
		template<typename T>
		T smoothStep(T value, T edge0, T edge1);

		/**
		 * @return absolute value.
		 */
		template<typename T>
		T abs(T value);

		/**
		 * Returns the machine epsilon, that is, the difference between 1.0 and the next value representable by the floating-point type T. 
		 * Only works for non-integral types.
		 * @return epsilon of type T.
		 */
		template<typename T>
		T constexpr epsilon();

		/**
		 * @return the maximum possible value of type T
		 */
		template<typename T>
		T constexpr max();
		
		/**
		 * @return the lowest finite value of type T
		 */
		template<typename T>
		T constexpr min();

		/**
		 * @param value requested sign value
		 * @return the sign of the given value, 1 for values > 0, -1 for values < 0, 0 for values of exactly 0.
		 */
		template<typename T>
		T sign(T value);

		/**
		 * @return a bell shaped curve based on value T (0-1)
		 * @param time sample value, from 0-1.
		 * @param strength exponent of the bell curve
		 */
		template<typename T>
		T bell(T time, T strength);

		/**
		 * Returns a random number of type T in the range of the given min / max value.
		 * Note that the random number is based on the seed set by setRandomSeed.
		 * @return a random number in range min / max. 
		 * @param min min random number
		 * @param max max random number
		 * @return the generated random number.
		 */
		template<typename T>
		T random(T min, T max);

		/**
		 * Sets the seed for all subsequent random calls.
		 * @param value the new seed value
		 */
		void NAPAPI setRandomSeed(int value);

		/**
		 * Interpolates a float value over time to a target using a dampening model
		 * @param currentValue the current blend value, often the return value
		 * @param targetValue the value to blend to
		 * @param currentVelocity the current velocity used to blend to target
		 * @param deltaTime time in seconds between cooks
		 * @param smoothTime approximately the time it will take to reach the target. A smaller value will reach the target faster.
		 * @param maxSpeed allows you to clamp the maximum speed
		 * @return the blended current value
		 */
		float NAPAPI smoothDamp(float currentValue, float targetValue, float& currentVelocity, float deltaTime, float smoothTime, float maxSpeed = math::max<float>());

		/**
		 * Interpolates a double value over time to a target using a dampening model.
		 * @param currentValue the current blend value, often the return value
		 * @param targetValue the value to blend to
		 * @param currentVelocity the current velocity used to blend to target
		 * @param deltaTime time in seconds between cooks
		 * @param smoothTime approximately the time it will take to reach the target. A smaller value will reach the target faster.
		 * @param maxSpeed allows you to clamp the maximum speed
		 * @return the blended current value
		 */
		double NAPAPI smoothDamp(double currentValue, double targetValue, double& currentVelocity, float deltaTime, float smoothTime, float maxSpeed = math::max<float>());

		/**
		* Smoothly interpolates a value of type T over time to a target using a dampening model.
		* This is a specialization of the default smoothDamp() function.
		* @param currentValue the current blend value, will be updated after calling
		* @param targetValue the value to blend to
		* @param currentVelocity the current velocity used to blend to target
		* @param deltaTime time in seconds between cooks
		* @param smoothTime approximately the time it will take to reach the target. A smaller value will reach the target faster.
		* @param maxSpeed allows you to clamp the maximum speed
		* @return the blended current value
		*/
		template<typename T>
		void smooth(T& currentValue, const T& targetValue, T& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);
		
		/**
		 * Composes a 4x4 matrix based on individual transformation, rotation and scale values
		 * @param translate a vector describing the objects position
		 * @param rotate quaternion describing the objects rotation
		 * @param scale a vector describing the objects scale
		 * @return the composed 4x4 matrix
		 */
		glm::mat4 NAPAPI composeMatrix(const glm::vec3& translate, const glm::quat& rotate, const glm::vec3& scale);

		/**
		 * Converts euler angles in to a quaternion
		 * The euler angle axis are interpreted as: roll(x), pitch(y), yaw(z), 
		 * @param eulerAngle the individual euler angles in radians
		 * @return the composed quaternion
		 */
		glm::quat NAPAPI eulerToQuat(const glm::vec3& eulerAngle);

		/**
		 * Converts euler angles in to a quaternion
		 * The euler angle axis are interpreted as: roll(x), pitch(y), yaw(z),
		 * @param roll the x axis rotation value in radians
		 * @param pitch the y axis rotation value in radians
		 * @param yaw the z axis rotation value in radians
		 * @return the composed quaternion
		 */
		glm::quat NAPAPI eulerToQuat(float roll, float pitch, float yaw);

		/**
		 * Converts an euler rotation in degrees to radians
		 * @param eulerDegrees the euler rotation roll(x), pitch(y), yaw(z) in degrees
		 * @return the euler rotation in radians
		 */
		glm::vec3 NAPAPI radians(const glm::vec3& eulerDegrees);

		/**
		 * Converts an euler rotation in degrees to radians
		 * The arguments are interpreted as floats
		 * @param roll the x axis rotation in degrees
		 * @param pitch the y axis rotation in degrees
		 * @param yaw the z axis rotation in degrees
		 * @return the euler rotation in radians
		 */
		glm::vec3 NAPAPI radians(float roll, float pitch, float yaw);

		/**
		 * Converts degrees to radians.
		 * @param degrees angle in degrees
		 * @return angle as radians
		 */
		float NAPAPI radians(float degrees);

		/**
		 * Converts radians to degrees.
		 * @param radians angle in radians
		 * @return angle in degrees
		 */
		float NAPAPI degrees(float radians);

		/**
		 * Extracts the position component from a 4x4 matrix.
		 * This call assumes the matrix is column major, the outermost array dimension is a column.
		 * @param matrix column major matrix
		 * @return the position component from a 4x4 matrix
		 */
		glm::vec3 NAPAPI extractPosition(const glm::mat4x4& matrix);

		/**
		 * Converts a point in object space to world space using the given object to world matrix.
		 * @param point the point in object space
		 * @param objectToWorldMatrix local to world space transformation matrix
		 * @return point in world space
		 */
		glm::vec3 NAPAPI objectToWorld(const glm::vec3& point, const glm::mat4x4& objectToWorldMatrix);

		/**
		 * Converts a point in world space to a point in object space using the given object to world matrix
		 * Note that this call internally uses matrix inversion, a rather costly operation
		 * @param point in world space
		 * @param objectToWorldMatrix the object to world transformation matrix
		 * @return point in object space
		 */
		glm::vec3 NAPAPI worldToObject(const glm::vec3& point, const glm::mat4x4& objectToWorldMatrix);

		/**
		 * Generate a UUID4, random every time, based on host device and random distribution
		 */
		std::string NAPAPI generateUUID();

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
		T constexpr epsilon()
		{
			return std::numeric_limits<T>::epsilon();
		}

		template<typename T>
		T constexpr max()
		{
			return std::numeric_limits<T>::max();
		}

		template<typename T>
		T constexpr min()
		{
			return std::numeric_limits<T>::lowest();
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

		template<typename T>
		T smoothStep(T value, T edge0, T edge1)
		{
			T x = math::clamp<T>((value - edge0) / (edge1 - edge0), 0, 1);
			return x * x * (3 - 2 * x);
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
		NAPAPI void smooth(double& currentValue, const double& targetValue, double& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI void smooth(glm::vec2& currentValue, const glm::vec2& targetValue, glm::vec2& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI void smooth(glm::vec3& currentValue, const glm::vec3& targetValue, glm::vec3& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI void smooth(glm::vec4& currentValue, const glm::vec4& targetValue, glm::vec4& currentVelocity, float deltaTime, float smoothTime, float maxSpeed);

		template<>
		NAPAPI uint random(uint min, uint max);

		template<>
		NAPAPI int random(int min, int max);

		template<>
		NAPAPI float random(float min, float max);

		template<>
		NAPAPI glm::vec3 random(glm::vec3 min, glm::vec3 max);

		template<>
		NAPAPI glm::vec4 random(glm::vec4 min, glm::vec4 max);

		template<>
		NAPAPI glm::vec2 random(glm::vec2 min, glm::vec2 max);

		template<>
		NAPAPI glm::ivec2 random(glm::ivec2 min, glm::ivec2 max);

		template<>
		NAPAPI glm::ivec3 random(glm::ivec3 min, glm::ivec3 max);

		template<>
		NAPAPI glm::ivec4 random(glm::ivec4 min, glm::ivec4 max);

		template<>
		NAPAPI glm::mat4 random(glm::mat4 min, glm::mat4 max);

		template<>
		NAPAPI float abs(float value);

		template<>
		NAPAPI int abs(int value);
	}
}
