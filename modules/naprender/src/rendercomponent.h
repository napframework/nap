#pragma once

// External Includes
#include <glm/glm.hpp>
#include <component.h>
#include <utility/dllexport.h>
#include <cameracomponent.h>

namespace nap
{
	// Forward declares
	class RenderableComponentInstance;

	/**
	 * Resource part of the render-able component. 
	 * Represents an object that can be rendered to screen or any other type of render target.
	 * This is the base class for all render-able types.
	 * Override the draw call to implement custom draw behavior.
	 */
	class NAPAPI RenderableComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderableComponent, RenderableComponentInstance)
	};


	/**
	 * Represents an object that can be rendered to screen or any other type of render target. 
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
		 * Draws this object to the currently active render target.
		 * @param viewMatrix often the camera world space location.
		 * @param projectionMatrix often the camera projection matrix.
		 */
		void draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

		/**
		 * Toggles visibility.
		 * @param visible if this object should be drawn or not
		 */
		void setVisible(bool visible)												{ mVisible = visible; }

		/**
		 * @return if the mesh is visible or not, default = true
		 */
		bool isVisible() const														{ return mVisible; }

		/**
		 * Called by the Render Service. By default every camera type is supported
		 * If your renderable component doesn't support a specific camera return false
		 * In that case the object won't be rendered.
		 * @return if the object can be rendered with the given camera
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const		{ return true; }

	protected:
		/**
		 * Draws the data to the currently active render target.
		 * Override this method to implement your own custom draw behavior.
		 * This method won't be called if the mesh isn't visible!
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) = 0;

	private:
		bool mVisible = true;			///< If this object should be drawn or not
	};
}
