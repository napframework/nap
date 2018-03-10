#pragma once

// External Includes
#include <glm/glm.hpp>
#include <component.h>
#include <utility/dllexport.h>

namespace nap
{
	// Forward declares
	class RenderableComponentInstance;

	/**
	 * Resource part of the render-able component.
	 * The instance of this component can be used to render something to screen or any other type of target. 
	 */
	class NAPAPI RenderableComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderableComponent, RenderableComponentInstance)
	};


	/**
	 * Represents an object that can be rendered to screen or any other type of target. 
	 * This is the base class for all render-able types.
	 * Override the draw call to implement custom draw behavior.
	 */
	class NAPAPI RenderableComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		RenderableComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{}

		/**
		 * Draws the data to the currently active render target
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) = 0;
	};
}
