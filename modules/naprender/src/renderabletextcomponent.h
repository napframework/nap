#pragma once

// Local Includes
#include "rendercomponent.h"
#include "material.h"

// External Includes
#include <font.h>

namespace nap
{
	class RenderableTextComponentInstance;

	/**
	 * Render-able Text Component Resource.
	 * Creates a RenderableTextComponentInstance that can draw text using a font and material.
	 */
	class NAPAPI RenderableTextComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableTextComponent, RenderableTextComponentInstance)
	public:

		/**
		 * This component requires a transform component to position the text
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<Font> mFont;								///< Property: 'Font' that represents the style of the text
		std::string mText;										///< Property: 'Text' to draw
		MaterialInstanceResource mMaterialInstanceResource;		///< Property: 'MaterialInstance' the material used to shade the text
	};


	/**
	 * Draws text into the currently active render target using a font and material
	 * This is the runtime version of the RenderableTextComponent resource.
	 */
	class NAPAPI RenderableTextComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderableTextComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableComponentInstance(entity, resource)									{ }

		/**
		 * Initialize renderabletextcomponentInstance based on the renderabletextcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the renderabletextcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update renderabletextcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Draws the text to the currently active render target
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix
		 */
		virtual void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;
	};
}
