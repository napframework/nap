// Local Includes
#include "cameracomponent.h"

// External Includes
#include <glm/gtc/matrix_transform.hpp>

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
		return glm::unProject(screenPos, getViewMatrix(), getProjectionMatrix(), glm::vec4(
			viewport.mMinPosition.x,
			viewport.mMinPosition.y,
			viewport.mMaxPosition.x - viewport.mMinPosition.x,
			viewport.mMaxPosition.y - viewport.mMinPosition.y));
	}


	glm::vec3 CameraComponentInstance::screenToWorld(const glm::vec2& screenPos, nap::RenderWindow& window)
	{
		glm::vec3 scre_pos = glm::vec3(screenPos.x, screenPos.y, 0.0f);
		window.makeActive();
		scre_pos.z = opengl::getDepth(screenPos.x, screenPos.y);
		return screenToWorld(scre_pos, window.getRect());
	}


	glm::vec3 CameraComponentInstance::worldToScreen(const glm::vec3& worldPos, math::Rect& viewport)
	{
		return glm::project(worldPos, getViewMatrix(), getProjectionMatrix(), glm::vec4(
			viewport.mMinPosition.x,
			viewport.mMinPosition.y,
			viewport.mMaxPosition.x - viewport.mMinPosition.x,
			viewport.mMaxPosition.y - viewport.mMinPosition.y));
	}


	glm::vec2 CameraComponentInstance::worldToScreen(const glm::vec3& worldPos, const nap::RenderWindow& window)
	{
		return worldToScreen(worldPos, window.getRect());
	}


	glm::vec3 CameraComponentInstance::rayFromScreen(const glm::vec2& screenPos, const math::Rect& viewport)
	{
		glm::vec4 ray_clip = glm::vec4(
			(2.0f * screenPos.x) / viewport.getWidth()  - 1.0f,
			(2.0f * screenPos.y) / viewport.getHeight() - 1.0f, 
			-1.0, 1.0);
		
		glm::vec4 ray_eye = glm::inverse(getProjectionMatrix()) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

		glm::vec3 ray_wor = glm::vec3(glm::inverse(getViewMatrix()) * ray_eye);
		return glm::normalize(ray_wor);
	}


	glm::vec3 CameraComponentInstance::rayFromScreen(const glm::vec2& screenPos, const nap::RenderWindow& window)
	{
		return rayFromScreen(screenPos, window.getRect());
	}

}