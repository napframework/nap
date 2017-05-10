#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/coremodule.h>
#include <glm/glm.hpp>


namespace nap
{
	/**
	 * Represents an object that can be rendered to screen
	 * or any other type of buffer. This is the base class
	 * for other render-able types.
	 * 
	 * You can override default draw behavior by specializing the draw method
	 */
	class RenderableComponent : public ServiceableComponent
	{
		RTTI_ENABLE(ServiceableComponent)
	public:

		/**
		 * Draws the data to the currently active render target
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) = 0;

	};
}
