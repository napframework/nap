#pragma once

#include "nap/entity.h"
#include "glm/glm.hpp"

namespace nap
{
	/**
	 * Base class for perspective and orthographic cameras.
	 */
	class CameraComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		CameraComponent(EntityInstance& entity);

		/**
		* @return camera projection matrix
		* Use this matrix to transform a 3d scene in to a 2d projection
		*/
		virtual const glm::mat4& getProjectionMatrix() const = 0;

		/**
		 * Returns the view matrix of the camera
		 * The view is determined by a number of factors including the camera's position
		 * and possible look at objects
		 * @return The populated view matrix
		 */
		virtual const glm::mat4 getViewMatrix() const = 0;
	};
}
