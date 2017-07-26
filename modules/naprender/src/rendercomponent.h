#pragma once

// External Includes
#include <glm/glm.hpp>
#include <nap/component.h>
#include <utility/dllexport.h>

namespace nap
{
	class NAPAPI RenderableComponentResource : public Component
	{
		RTTI_ENABLE(Component)
	};

	/**
	 * Represents an object that can be rendered to screen
	 * or any other type of buffer. This is the base class
	 * for other render-able types.
	 * 
	 * You can override default draw behavior by specializing the draw method
	 */
	class RenderableComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		RenderableComponentInstance(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		/**
		 * Draws the data to the currently active render target
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) = 0;
	};
}
