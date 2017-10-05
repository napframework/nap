// Local Includes
#include "mathutils.h"

// External Inlcudes
#include <memory>
#include <random>

// Specialization of lerping
namespace nap
{
	namespace math
	{
		int random(int min, int max)
		{
			static std::unique_ptr<std::mt19937> generator = nullptr;
			if (generator == nullptr)
			{
				std::random_device r;
				generator = std::make_unique<std::mt19937>(r());
			}
			std::uniform_int_distribution<int> m_distribution(min, max);
			return m_distribution(*generator);
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
	}
}