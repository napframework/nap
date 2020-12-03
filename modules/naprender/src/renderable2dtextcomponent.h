/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "renderabletextcomponent.h"

// External Includes
#include <fontutils.h>

namespace nap
{
	// Forward Declares
	class Renderable2DTextComponentInstance;
	class RenderService;

	/**
	 * Allows you to render text at a specific location in screen space.
	 * Use this component when you want to render text at a specific location on screen or in a render-target.
	 * Use the Renderable3DTextComponent to draw text in 3D space with a perspective camera.
	 *
	 * Call draw() in the render part of your application to render text to a specific location on screen or a render-target.
	 * It is also possible to render the text using RenderService::renderObjects(), this is similar to how meshes are rendered.
	 * In that case the x/y location of the camera influences the final location of the text.
	 *
	 * When the parent entity has a transform component attached to it and 'IgnoreTransform' is set to false, 
	 * the x/y Translate values are used as offset in pixel space.
	 * 2D text cannot be scaled or rotated, this ensures that every Glyph is rendered in it's native resolution.
	 * When rendering this component through the render interface of the render service it is advised to use an orthographic camera.
	 *
	 * It is possible to cache multiple lines at once, where each line can be selected and drawn individually inside a render loop.
	 * This is useful when you want the same component to render multiple lines of text, removing the need to declare a component for each individual line.
	 * You cannot update or add a line of text when rendering a frame: inside the render loop.
	 * Only update or add new lines of text on update. You can however change the position and line of text to draw inside the render loop.
	 *
	 * For example, on update:
	 * ~~~~~
	 *		// Set text for next draw operation
	 *		text_component.resize(blobs.size());
	 *		for (int i = 0; i < blobs.size(); i++)
	 *		{
	 *			text_component.setText(i, utility::stringFormat("Blob %d", i + 1), error);
	 *		}
	 * ~~~~~
	 *
	 * And on render:
	 * ~~~~~
	 *		// Set text for next draw operation
	 *		for (int i = 0; i < blobs.size(); i++)
	 *		{
	 *			// Get blob location in screen space
	 *			glm::vec3 blob_pos = locs[i];
	 *			glm::vec2 text_pos = persp_camera.worldToScreen(blob_pos, mRenderWindow->getRectPixels());
	 *
	 *			// Set location, select line and draw
	 *			text_comp.setLocation(text_pos);
	 *			text_comp.setLineIndex(i);
	 *			text_comp.draw(*mRenderWindow)
	 *		}
	 * ~~~~~
	 *
	 */
	class NAPAPI Renderable2DTextComponent : public RenderableTextComponent
	{
		RTTI_ENABLE(RenderableTextComponent)
		DECLARE_COMPONENT(Renderable2DTextComponent, Renderable2DTextComponentInstance)

	public:
		utility::ETextOrientation mOrientation = utility::ETextOrientation::Left;	///< Property: 'Orientation' Text draw orientation
		glm::ivec2 mLocation = { 0,0 };												///< Property: 'Location' text location in pixel coordinates
		EDepthMode mDepthMode = EDepthMode::NoReadWrite;							///< Property: 'DepthMode' how text is handled by z-buffer.
		bool mIgnoreTransform = true;												///< Property: 'IgnoreTransform' if the transform is ignored when present
	};


	/**
	 * Allows you to render text at a specific location in screen space.
	 * Use this component when you want to render text at a specific location on screen or in a render-target.
	 *
	 * Call draw() in the render part of your application to render text to a specific location on screen or a render-target.
	 * It is also possible to render the text using RenderService::renderObjects(), this is similar to how meshes are rendered.
	 * In that case the x/y location of the camera influences the final location of the text.

	 * When the parent entity has a transform component attached to it and 'IgnoreTransform' is set to false, 
	 * the x/y Translate values are used as offset in pixel space.
	 * 2D text cannot be scaled or rotated, this ensures that every Glyph is rendered in it's native resolution.
	 * When rendering this component through the render interface of the render service it is advised to use an orthographic camera.
	 *
	 * It is possible to cache multiple lines at once, where each line can be selected and drawn individually inside a render loop.
	 * This is useful when you want the same component to render multiple lines of text, removing the need to declare a component for each individual line.
	 * You cannot update or add a line of text when rendering a frame: inside the render loop.
	 * Only update or add new lines of text on update. You can however change the position and line of text to draw inside the render loop.
	 *
	 * For example, on update:
	 * ~~~~~
	 *		// Set text for next draw operation
	 *		text_component.resize(blobs.size());
	 *		for (int i = 0; i < blobs.size(); i++)
	 *		{
	 *			text_component.setText(i, utility::stringFormat("Blob %d", i + 1), error);
	 *		}
	 * ~~~~~
	 *
	 * And on render:
	 * ~~~~~
	 *		// Set text for next draw operation
	 *		for (int i = 0; i < blobs.size(); i++)
	 *		{
	 *			// Get blob location in screen space
	 *			glm::vec3 blob_pos = locs[i];
	 *			glm::vec2 text_pos = persp_camera.worldToScreen(blob_pos, mRenderWindow->getRectPixels());
	 *
	 *			// Set location, select line and draw
	 *			text_comp.setLocation(text_pos);
	 *			text_comp.setLineIndex(i);
	 *			text_comp.draw(*mRenderWindow)
	 *		}
	 * ~~~~~
	 *
	 */
	class NAPAPI Renderable2DTextComponentInstance : public RenderableTextComponentInstance
	{
		RTTI_ENABLE(RenderableTextComponentInstance)
	public:
		Renderable2DTextComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableTextComponentInstance(entity, resource)									{ }

		/**
		 * Initialize Renderable2DTextComponentInstance based on the Renderable2DTextComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the Renderable2DTextComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Draws the current text into the active target using the stored coordinates in screen space.
		* This is a convenience function that calls RenderableTextComponentInstance::draw using a custom view and projection matrix.
		* These matrices are created based on on the size of your render target.
		* Only Call this function in the render part of your application. Set text in the update part of your application.
		* The x and y coordinates of the TransformComponent are used as an offset (in pixels) if the parent entity has a transform component.
		* When using this function the orientation of the text is taken into account.
		* @param target render target that defines the screen space bounds
		*/
		void draw(IRenderTarget& target);

		/**
		 * @return current text draw orientation
		 */
		utility::ETextOrientation getOrientation() const					{ return mOrientation; }

		/**
		 * Sets the text draw orientation
		 * @param orientation new text orientation
		 */
		void setOrientation(utility::ETextOrientation orientation)			{ mOrientation = orientation; }

		/**
		 * Returns the current text coordinates in screen space. This is not the final position of the text!
		 * To get the actual start position of the text use getTextPosition()
		 * The final location is based on the orientation and transformation of this component.
		 * @return location of the text in screen space pixel coordinates
		 */
		const glm::ivec2& getLocation() const								{ return mLocation; }

		/**
		 * Sets the new text coordinates in screen space. This is not the final position of the text!
		 * To get the actual start position of the text use getTextPosition()
		 * The final location is based on the orientation and transformation of this component.
		 * @param coordinates the new text location in pixel space coordinates
		 */
		void setLocation(const glm::ivec2& coordinates)						{ mLocation = coordinates; }

		/**
		* Returns the text position based on the stored location, orientation mode and text transformation.
		* The return value is the actual position of the text in the world and most likely screen.
		* Actual screen space position still depends on camera location, orientation and shader effects.
		* @return start position of the text based based on stored location, orientation mode and text transformation.
		*/
		glm::ivec2 getTextPosition();

		/**
		 * Creates a Renderable2DGlyph for the given index in the font.
		 * @param index the index to create the renderable glyph for.
		 * @param error contains the error if the glyph representation could not be created.
		 * @return the Renderable2DGlyph glyph for the given character index.
		 */
		virtual RenderableGlyph* getRenderableGlyph(uint index, utility::ErrorState& error) const override;

		/**
		 * This component can only be rendered with an orthographic camera!
		 * @return if the camera is an orthographic camera or not.
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override;

	protected:
		/**
		 * Draws the text to the currently active render target using the render service.
		 * This function is called by the render service when text is rendered with a user defined camera.
		 * In that case the viewMatrix is the world space camera location and the the projection matrix is defined by the camera type.
		 * You can only use an orthographic camera when rendering 2D text.
		 * The x/y location of the parent entity is taken into account if there is a transform component.
		 * Note that the orientation mode is also taken into account when rendering this way.
		 * @param renderTarget bound target to render to
		 * @param commandBuffer current command buffer
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix, orthographic or perspective
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		utility::ETextOrientation mOrientation = utility::ETextOrientation::Left;

		/**
		 * Computes object space text matrix based on the stored coordinates
		 * @param outMatrix the computed text model matrix
		 */
		void computeTextModelMatrix(glm::mat4x4& outMatrix);

		glm::ivec2		mLocation = { 0,0 };		///< Text location in pixel coordinates
		RenderService*	mService = nullptr;			///< Render service
		bool			mIgnoreTransform = true;	///< If transform should be ignored when present
	};
}
