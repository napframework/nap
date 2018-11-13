#pragma once

// Local Includes
#include "renderabletextcomponent.h"

// External Includes
#include <component.h>

namespace nap
{
	class Renderable3DTextComponentInstance;

	/**
	 * Draws flat text in 3D space.
	 * Use this component when you want to render text at a specific location in the world
	 * Use the Renderable2DTextComponent to draw text in screen (pixel) space with an orthographic camera.
	 */
	class NAPAPI Renderable3DTextComponent : public RenderableTextComponent
	{
		RTTI_ENABLE(RenderableTextComponent)
		DECLARE_COMPONENT(Renderable3DTextComponent, Renderable3DTextComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * Runtime version of the Renderable3DTextComponent.
	 * This component allows you to render a single line of text at a specific location in the world.
	 * 3D text can only be rendered using the render service, similar to how 3D meshes are rendered.
	 * The text can be transformed, scaled and rotated. It's best to render 3D text using a perspective camera.
	 * The font size directly influences the size of the text. 
	 * Use the bounding box to position and scale the text.
	 */
	class NAPAPI Renderable3DTextComponentInstance : public RenderableTextComponentInstance
	{
		RTTI_ENABLE(RenderableTextComponentInstance)
	public:
		Renderable3DTextComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableTextComponentInstance(entity, resource)								{ }

		/**
		 * Initialize Renderable3DTextComponentInstance based on the Renderable3DTextComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the Renderable3DTextComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	protected:
		/**
		 * Draws the text to the currently active render target using the render service.
		 * The size of the text is directly related to the size of the font.
		 * This function is called by the render service when text is rendered with a user defined camera.
		 * In that case the viewMatrix is the world space camera location and the the projection matrix is defined by the camera type.
		 * This can be orthographic or perspective. It is recommended to only use a perspective camera when rendering text in 3D.
		 * The TransformComponent of the parent entity is used to place the text and is therefore required.
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix, orthographic or perspective
		 */
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;
	};
}
