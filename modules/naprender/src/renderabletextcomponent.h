#pragma once

// Local Includes
#include "rendercomponent.h"
#include "material.h"
#include "renderglobals.h"
#include "transformcomponent.h"

// External Includes
#include <font.h>
#include <planemesh.h>
#include <renderablemesh.h>

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
		std::string mGlyphUniform = glyphUniform;				///< Property: 'GlyphUniform' name of the 2D texture character binding in the shader
	};


	/**
	 * Draws text into the currently active render target using a font and material
	 * This is the runtime version of the RenderableTextComponent resource.
	 */
	class NAPAPI RenderableTextComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
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
		 * @return the font used to display text.
		 */
		const FontInstance& getFont() const;

		/**
		 * Set the text to be drawn
		 * @param text the new line of text to draw
		 */
		void setText(const std::string& text)				{ mText = text; }

		/**
		 * @return the text that is drawn.
		 */
		const std::string& getText() const					{ return mText; }

		/**
		 * @return material used when drawing the text.
		 */
		MaterialInstance& getMaterialInstance();

	protected:
		/**
		 * Draws the text to the currently active render target
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix
		 */
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		FontInstance* mFont = nullptr;							///< Pointer to the font, set on initialization
		std::string mText = "";									///< Text to render
		MaterialInstance mMaterialInstance;						///< The MaterialInstance as created from the resource. 
		PlaneMesh mPlane;										///< Plane used to draws a single letter
		std::string mGlyphUniform = glyphUniform;				///< Name of the 2D texture character binding in the shader
		TransformComponentInstance* mTransform = nullptr;		///< Transform used to position text
		RenderableMesh mRenderableMesh;							///< Valid Plane / Material combination
		VertexAttribute<glm::vec3>* mPositionAttr = nullptr;	///< Handle to the plane vertices
	};
}
