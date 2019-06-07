#pragma once

// External Includes
#include <glm/glm.hpp>
#include <limits>
#include <utility/dllexport.h>
#include <algorithm>
#include <string>

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
		 * Rounds down a value
		 * @return floored value of T
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
		 * @return absolute value of T
		 */
		template<typename T>
		T abs(T value);

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
		template<typename T>
		T random(T min, T max);

		/**
		 * Sets the seed for all subsequent random calls.
		 * @param value the new seed value
		 */
		void NAPAPI setRandomSeed(int value);

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
		 * @param the euler rotation roll(x), pitch(y), yaw(z) in degrees
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
		 * Converts degrees to radians
		 * @param degrees angle in degrees
		 * @return angle as radians
		 */
		float NAPAPI radians(float degrees);

		/**
		 * Converts radians to degrees
		 * @param radians angle in radians
		 * @return angle in degrees
		 */
		float NAPAPI degrees(float radians);

		/**
		 * Extracts the position component from a 4x4 matrix.
		 * This call assumes the matrix is column major, ie: the outermost array dimension is a column
		 * @param matrix column major matrix
		 * @return the position component from a 4x4 matrix
		 */
		glm::vec3 NAPAPI extractPosition(const glm::mat4x4& matrix);

		/**
		 * Converts a point in object space to world space using the given object to world matrix
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
		 * Generates a unique identifier.
		 * TODO: Implement proper UUID generation, this is temp solution based on a random number generator.
		 * @return a uuid as string
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
		NAPAPI int lerp<int>(const int& start, const int& end, float percent);

		template<>
		NAPAPI glm::vec2 lerp<glm::vec2>(const glm::vec2& start, const glm::vec2& end, float percent);

		template<>
		NAPAPI glm::vec3 lerp<glm::vec3>(const glm::vec3& start, const glm::vec3& end, float percent);

		template<>
		NAPAPI glm::vec4 lerp<glm::vec4>(const glm::vec4& start, const glm::vec4& end, float percent);

		template<>
		NAPAPI glm::ivec4 lerp<glm::ivec4>(const glm::ivec4& start, const glm::ivec4& end, float percent);

		template<>
		NAPAPI glm::ivec3 lerp<glm::ivec3>(const glm::ivec3& start, const glm::ivec3& end, float percent);

		template<>
		NAPAPI glm::ivec2 lerp<glm::ivec2>(const glm::ivec2& start, const glm::ivec2& end, float percent);

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
		NAPAPI float abs(float value);

		template<>
		NAPAPI int abs(int value);
	}
}
