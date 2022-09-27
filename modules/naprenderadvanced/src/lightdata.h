#pragma once

// External includes
#include <mathutils.h>

namespace nap
{
	/**
	 * Directional Light
	 */
	struct DirectionalLightData
	{
		glm::vec3 direction;
		glm::vec3 color;
		float intensity;
	};

	/**
	 * Point light
	 */
	struct PointLightData
	{
		glm::vec3 origin;
		glm::vec3 direction;
		glm::vec3 color;
		float intensity;
	};

	/**
	 * Spot Light
	 */
	struct SpotLightData
	{
		glm::vec3 origin;
		glm::vec3 direction;
		glm::vec3 color;
		float innerAngle;		// Light intensity is at maximum from center until inner angle of the cone
		float outerAngle;		// Light intensity falls off towards the outer angle of the cone
		float radius;			// The maximum radius of the cone
	};
}
