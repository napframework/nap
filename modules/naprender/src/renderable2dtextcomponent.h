#pragma once

// Local Includes
#include "renderabletextcomponent.h"

// External Includes
#include <fontutils.h>

namespace nap
{
	class Renderable2DTextComponentInstance;

	/**
	 * Draws text to screen in screen space (pixel) coordinates.
	 * Use this component when you want to render text at a specific location on screen or in a render-target.
	 * Use the Renderable3DTextComponent to draw text in 3D space with a perspective camera.
	 */
	class NAPAPI Renderable2DTextComponent : public RenderableTextComponent
	{
		RTTI_ENABLE(RenderableTextComponent)
		DECLARE_COMPONENT(Renderable2DTextComponent, Renderable2DTextComponentInstance)

	public:
		utility::ETextOrientation mOrientation = utility::ETextOrientation::Left;		///< Property: 'Orientation' Text draw orientation
	};


	/**
	 * Runtime version of the Renderable2DTextComponent.
	 * This component allows you to render a single line of text to screen at a specific location in pixel space.
	 * Call draw() in the render part of your application to render text to a specific location on screen or a render-target.
	 * It is also possible to render the text using RenderService::renderObjects(), this is similar to how meshes are rendered.
	 * In that case the camera location influences the final location of the text.
	 * When the parent entity has a transform component attached to it the x/y Translate values are used as text offset in pixel space.
	 * 2D text cannot be scaled or rotated, this ensures that every Glyph is rendered in it's native resolution.
	 * When rendering this component through the render interface of the render service it is advised to use an orthographic camera.
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
		* Draws the current text into the active target using the provided coordinates in screen space.
		* This is a convenience function that calls RenderableTextComponentInstance::draw using a custom view and projection matrix.
		* These matrices are created based on on the size of your render target.
		* Only Call this function in the render part of your application. Set text in the update part of your application.
		* The x and y coordinates of the TransformComponent are used as an offset (in pixels) if the parent entity has a transform component.
		* When using this function the orientation of the text is taken into account.
		* @param coordinates the location of the text in screen space, 0,0 = lower left corner
		* @param target render target that defines the screen space bounds
		*/
		void draw(const glm::ivec2& coordinates, const opengl::BackbufferRenderTarget& target);

		/**
		* Draws the text to the currently active render target using the render service.
		* This function is called by the render service when text is rendered with a user defined camera.
		* In that case the viewMatrix is the world space camera location and the the projection matrix is defined by the camera type.
		* This can be orthographic or perspective. It is recommended to only use an orthographic camera when rendering 2D text.
		* The x/y location of the parent entity is taken into account if there is a transform component.
		* Note that the orientation mode is also taken into account when rendering this way.
		* @param viewMatrix the camera world space location
		* @param projectionMatrix the camera projection matrix, orthographic or perspective
		*/
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

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
		* Computes the text location based on the given origin, orientation mode and bounding box.
		* @param origin reference point in 2D space.
		* @return position of the text based on the given origin, orientation mode and bounding box
		*/
		glm::ivec2 getTextPosition(const glm::ivec2& origin);

		/**
		 * Creates a Renderable2DGlyph for the given index in the font.
		 * @param index the index to create the renderable glyph for.
		 * @param error contains the error if the glyph representation could not be created.
		 * @return the Renderable2DGlyph glyph for the given character index.
		 */
		virtual RenderableGlyph* getRenderableGlyph(uint index, utility::ErrorState& error) const override;

	private:
		utility::ETextOrientation mOrientation = utility::ETextOrientation::Left;

		/**
		 * Computes object space text matrix based on the given coordinates
		 * @param coordinates pixel coordinates where the text should be drawn 
		 * @param outMatrix the computed text model matrix
		 */
		void computeTextModelMatrix(const glm::ivec2& coordinates, glm::mat4x4& outMatrix);
	};
}
