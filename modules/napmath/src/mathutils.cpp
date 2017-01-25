// Local Includes
#include "mathutils.h"

// External Includes
#include <glm/gtc/quaternion.hpp>


namespace nap
{
	// Returns a quaternion as a vector struct
	glm::vec4 quatToVector(const glm::quat& quaternion)
	{
		return glm::vec4({ quaternion.x, quaternion.y, quaternion.z, quaternion.w });
	}


	// Returns vector as quaternion
	glm::quat vectorToQuat(const glm::vec4& vector)
	{
		return glm::quat({ vector.x, vector.y, vector.z, vector.w });
	}
}