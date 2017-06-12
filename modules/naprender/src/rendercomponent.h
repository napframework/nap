#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/coremodule.h>
#include <glm/glm.hpp>
#include "nap/entity.h"


namespace nap
{
	class RenderableComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)
	};

	/**
	 * Represents an object that can be rendered to screen
	 * or any other type of buffer. This is the base class
	 * for other render-able types.
	 * 
	 * You can override default draw behavior by specializing the draw method
	 */
	class RenderableComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		RenderableComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		/**
		 * Draws the data to the currently active render target
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) = 0;
	};
}
