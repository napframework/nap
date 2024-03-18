/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <glm/glm.hpp>
#include <component.h>
#include <utility/dllexport.h>
#include <cameracomponent.h>

// Local includes
#include "rendertag.h"
#include "renderlayer.h"

namespace nap
{
	// Forward declares
	class RenderableComponentInstance;
	class RenderService;

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

	public:
		bool mVisible = true;								///< Property: 'Visible' if this object is rendered to target by the render service.
		ResourcePtr<RenderLayer> mLayer;					///< Property: 'Layer' the render layer assigned to this component 
		std::vector<ResourcePtr<RenderTag>> mTags;			///< Property: 'Tags' List of tags specifying the category this render component belongs to.
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
		RenderableComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Make sure to this in derived classes
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Called by the render service, calls onDraw() if visible.
		 * Renders the object to the given render target using the provided command buffer, view and projection matrix.
		 * @param renderTarget target to render to
		 * @param commandBuffer active command buffer
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
		 * @return the list of tags
		 */
		const std::vector<ResourcePtr<RenderTag>>& getTags() const					{ return getComponent<RenderableComponent>()->mTags; }

		/**
		 * @return the render mask
		 */
		RenderMask getMask() const													{ return mRenderMask; }

		/**
		 * @return if this component is compatible with (includes) the given mask
		 */
		bool hasMask(RenderMask otherMask)											{ return (mRenderMask == 0) || ((mRenderMask & otherMask) > 0); }

		/**
		 * Returns the rank of this component in the render chain, defaults to 0 (front) if no layer is assigned.
		 * The rank controls the order in which components are rendered, where 0 is the front and the last index is the back.
		 * @return the rank index in the render chain, 0 (front) if no layer is assigned
		 */
		int getRank() const;

		/**
		 * Returns the layer assigned to this component.
		 * Every layer is assigned a rank which controls the order in which components are rendered, where 0 is the front and the last index is the back.
		 * @return the render layer, nullptr if no layer is given
		 */
		const RenderLayer* getLayer() const { return mRenderLayer; }

		/**
		 * Called by the Render Service. By default every camera type is supported
		 * If your renderable component doesn't support a specific camera return false
		 * In that case the object won't be rendered.
		 * @return if the object can be rendered with the given camera
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const		{ return true; }

	protected:
		/**
		 * Called by the render service.
		 * Override this method to implement your own custom draw behavior.
		 * This method won't be called if the mesh isn't visible.
		 * @param renderTarget currently bound render target
		 * @param commandBuffer active command buffer
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) = 0;

		RenderService* mRenderService = nullptr;

	private:
		bool mVisible = true;							///< If this object should be drawn or not
		RenderLayer* mRenderLayer = nullptr;			///< The render layer
		RenderMask mRenderMask = 0;						///< The render mask
	};
}
