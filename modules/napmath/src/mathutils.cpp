// Local Includes
#include "mathutils.h"

// Specialization of lerping
namespace nap
{
	namespace math
	{
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
	}
}