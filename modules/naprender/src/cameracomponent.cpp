// Local Includes
#include "cameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CameraComponentInstance)
RTTI_END_CLASS

namespace nap
{
	CameraComponentInstance::CameraComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	glm::vec3 CameraComponentInstance::screenToWorld(const glm::vec3& screenPos, const math::Rect& viewport)
	{
		setRenderTargetSize({ viewport.getWidth(), viewport.getHeight() });
		return glm::unProject(screenPos, getViewMatrix(), getProjectionMatrix(), glm::vec4(
			viewport.mMinPosition.x,
			viewport.mMinPosition.y,
			viewport.mMaxPosition.x - viewport.mMinPosition.x,
			viewport.mMaxPosition.y - viewport.mMinPosition.y));
	}


	glm::vec3 CameraComponentInstance::worldToScreen(const glm::vec3& worldPos, const math::Rect& viewport)
	{
		setRenderTargetSize({ viewport.getWidth(), viewport.getHeight() });
		return glm::project(worldPos, getViewMatrix(), getProjectionMatrix(), glm::vec4(
			viewport.mMinPosition.x,
			viewport.mMinPosition.y,
			viewport.mMaxPosition.x - viewport.mMinPosition.x,
			viewport.mMaxPosition.y - viewport.mMinPosition.y));
	}


	glm::vec3 CameraComponentInstance::rayFromScreen(const glm::vec2& screenPos, const math::Rect& viewport)
	{
		setRenderTargetSize({ viewport.getWidth(), viewport.getHeight() });
		glm::vec4 ray_clip = glm::vec4(
			(2.0f * screenPos.x) / viewport.getWidth()  - 1.0f,
			(2.0f * screenPos.y) / viewport.getHeight() - 1.0f, 
			-1.0, 1.0);

		glm::vec4 ray_eye = glm::inverse(getProjectionMatrix()) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

		glm::vec3 ray_wor = glm::vec3(glm::inverse(getViewMatrix()) * ray_eye);
		return glm::normalize(ray_wor);
	}
}