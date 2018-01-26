// Local Includes
#include "mathutils.h"

// External Inlcudes
#include <memory>
#include <random>
#include <cmath>

// Specialization of lerping
namespace nap
{
	namespace math
	{
		double pi()
		{
			return M_PI;
		}


		static std::mt19937& getGenerator()
		{
			static std::unique_ptr<std::mt19937> generator = nullptr;
			if (generator == nullptr)
			{
				std::random_device r;
				generator = std::make_unique<std::mt19937>(r());
			}
			return *generator;
		}


		static int randomInt(int min, int max)
		{
			std::uniform_int_distribution<int> m_distribution(min, max);
			return m_distribution(getGenerator());
		}


		static float randomFloat(float min, float max)
		{
			std::uniform_real_distribution<float> m_distribution(min, max);
			return m_distribution(getGenerator());
		}


		template<>
		float lerp<float>(const float& start, const float& end, float percent)
		{
			return glm::mix<float>(start, end, percent);
		}


		template<>
		glm::vec4 lerp<glm::vec4>(const glm::vec4& start, const glm::vec4& end, float percent)
		{
			glm::vec4 return_v;
			return_v.x = lerp<float>(start.x, end.x, percent);
			return_v.y = lerp<float>(start.y, end.y, percent);
			return_v.z = lerp<float>(start.z, end.z, percent);
			return_v.w = lerp<float>(start.w, end.w, percent);
			return return_v;
		}


		template<>
		glm::vec3 lerp<glm::vec3>(const glm::vec3& start, const glm::vec3& end, float percent)
		{
			glm::vec3 return_v;
			return_v.x = lerp<float>(start.x, end.x, percent);
			return_v.y = lerp<float>(start.y, end.y, percent);
			return_v.z = lerp<float>(start.z, end.z, percent);
			return return_v;
		}


		template<>
		glm::vec2 lerp<glm::vec2>(const glm::vec2& start, const glm::vec2& end, float percent)
		{
			glm::vec2 return_v;
			return_v.x = lerp<float>(start.x, end.x, percent);
			return_v.y = lerp<float>(start.y, end.y, percent);
			return return_v;
		}


		template<>
		double lerp<double>(const double& start, const double& end, float percent)
		{
			return glm::mix<double>(start, end, percent);
		}


		template<>
		double power<double>(double value, double exp)
		{
			return pow(value, exp);
		}


		template<>
		float power<float>(float value, float exp)
		{
			return pow(value, exp);
		}


		template<>
		int power<int>(int value, int exp)
		{
			return static_cast<int>(power<float>(static_cast<float>(value), static_cast<float>(exp)));
		}


		float smoothDamp(float currentValue, float targetValue, float& currentVelocity, float deltaTime, float smoothTime, float maxSpeed)
		{
			smoothTime = math::max<float>(0.0001f, smoothTime);
			float num = 2.0f / smoothTime;
			float num2 = num * deltaTime;
			float num3 = 1.0f / (1.0f + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2 * num2);
			float num4 = currentValue - targetValue;
			float num5 = targetValue;
			float num6 = maxSpeed * smoothTime;
			num4 = math::clamp<float>(num4, -num6, num6);
			float target = currentValue - num4;
			float num7 = (currentVelocity + num * num4) * deltaTime;
			currentVelocity = (currentVelocity - num * num7) * num3;
			float num8 = target + (num4 + num7) * num3;
			if (num5 - currentValue > 0.0f == num8 > num5)
			{
				num8 = num5;
				currentVelocity = (num8 - num5) / deltaTime;
			}
			return num8;
		}


		template<>
		void smooth(float& currentValue, const float& targetValue, float& currentVelocity, float deltaTime, float smoothTime, float maxSpeed)
		{
			currentValue = smoothDamp(currentValue, targetValue, currentVelocity, deltaTime, smoothTime, maxSpeed);
		}


		template<>
		void smooth(glm::vec2& currentValue, const glm::vec2& targetValue, glm::vec2& currentVelocity, float deltaTime, float smoothTime, float maxSpeed)
		{
			currentValue.x = smoothDamp(currentValue.x, targetValue.x, currentVelocity.x, deltaTime, smoothTime, maxSpeed);
			currentValue.y = smoothDamp(currentValue.y, targetValue.y, currentVelocity.y, deltaTime, smoothTime, maxSpeed);
		}


		template<>
		void smooth(glm::vec3& currentValue, const glm::vec3& targetValue, glm::vec3& currentVelocity, float deltaTime, float smoothTime, float maxSpeed)
		{
			currentValue.x = smoothDamp(currentValue.x, targetValue.x, currentVelocity.x, deltaTime, smoothTime, maxSpeed);
			currentValue.y = smoothDamp(currentValue.y, targetValue.y, currentVelocity.y, deltaTime, smoothTime, maxSpeed);
			currentValue.z = smoothDamp(currentValue.z, targetValue.z, currentVelocity.z, deltaTime, smoothTime, maxSpeed);
		}


		template<>
		void smooth(glm::vec4& currentValue, const glm::vec4& targetValue, glm::vec4& currentVelocity, float deltaTime, float smoothTime, float maxSpeed)
		{
			currentValue.x = smoothDamp(currentValue.x, targetValue.x, currentVelocity.x, deltaTime, smoothTime, maxSpeed);
			currentValue.y = smoothDamp(currentValue.y, targetValue.y, currentVelocity.y, deltaTime, smoothTime, maxSpeed);
			currentValue.z = smoothDamp(currentValue.z, targetValue.z, currentVelocity.z, deltaTime, smoothTime, maxSpeed);
			currentValue.w = smoothDamp(currentValue.w, targetValue.w, currentVelocity.w, deltaTime, smoothTime, maxSpeed);
		}


		glm::vec3 extractPosition(const glm::mat4x4& matrix)
		{
			return{ matrix[3][0], matrix[3][1], matrix[3][2] };
		}


		glm::mat4 NAPAPI objectToWorld(const glm::mat4x4& objectSpace, const glm::mat4x4& transform)
		{
			return transform * objectSpace;
		}


		template<>
		int random(int min, int max)
		{
			return randomInt(min, max);
		}


		template<>
		float random(float min, float max)
		{
			return randomFloat(min, max);
		}


		template<>
		glm::vec3 random(glm::vec3 min, glm::vec3 max)
		{
			return
			{
				randomFloat(min.x, max.x),
				randomFloat(min.y, max.y),
				randomFloat(min.z, max.z)
			};
		}


		template<>
		glm::vec4 random(glm::vec4 min, glm::vec4 max)
		{
			return
			{
				randomFloat(min.x, max.x),
				randomFloat(min.y, max.y),
				randomFloat(min.z, max.z),
				randomFloat(min.w, max.w)
			};
		}


		template<>
		glm::vec2 random(glm::vec2 min, glm::vec2 max)
		{
			return
			{
				randomFloat(min.x, max.x),
				randomFloat(min.y, max.y)
			};
		}


		template<>
		glm::ivec3 random(glm::ivec3 min, glm::ivec3 max)
		{
			return
			{
				randomInt(min.x, max.x),
				randomInt(min.y, max.y),
				randomInt(min.z, max.z)
			};
		}


		template<>
		glm::ivec4 random(glm::ivec4 min, glm::ivec4 max)
		{
			return
			{
				randomInt(min.x, max.x),
				randomInt(min.y, max.y),
				randomInt(min.z, max.z),
				randomInt(min.w, max.w)
			};
		}


		template<>
		glm::ivec2 random(glm::ivec2 min, glm::ivec2 max)
		{
			return
			{
				randomInt(min.x, max.x),
				randomInt(min.y, max.y)
			};
		}
	}
}