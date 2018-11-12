#pragma once

// Local Includes
#include "rendercomponent.h"
#include "material.h"
#include "renderglobals.h"
#include "transformcomponent.h"
#include "renderableglyph.h"

// External Includes
#include <font.h>
#include <planemesh.h>
#include <renderablemesh.h>
#include <nbackbufferrendertarget.h>

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

		/**
		 * Horizontal text draw orientation, vertical alignment is based on character origin
		 */
		enum class EOrientation : int
		{
			Left	= 0,		///< Draws text to the right of the horizontal coordinate
			Center	= 1,		///< Centers the text around the horizontal coordinate
			Right	= 2			///< Draws the text to the left of the horizontal coordinate
		};

		RenderableTextComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableComponentInstance(entity, resource)									{ }

		/**
		 * Initializes the this component.
		 * @param errorState holds the error message when initialization fails
		 * @return if the component initialized successfully
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
		 * Set the text to be drawn. Only set or change text on app update, not render.
		 * @param text the new line of text to draw.
		 * @param error list of unsupported characters
		 * @return if all characters in the text are supported and can be drawn.
		 */
		bool setText(const std::string& text, utility::ErrorState& error);

		/**
		 * @return the text that is drawn.
		 */
		const std::string& getText() const					{ return mText; }

		/**
		 * @return material used when drawing the text.
		 */
		MaterialInstance& getMaterialInstance();

		/**
		 * Draws the current text into the active target using the provided coordinates in screen space.
		 * This is a convenience function that calls draw() using a view and projection matrix based on the size of your render target.
		 * Call this function in the render part of your application.
		 * The current location of this component is still taken into account and acts as a pixel offset
		 * @param coordinates the location of the text in screen space
		 * @param target render target that defines the screen space bounds
		 * @param orientation how to position the text.
		 */
		void draw(glm::ivec2 coordinates, const opengl::BackbufferRenderTarget& target, EOrientation orientation = EOrientation::Left);

		/**
		 * @return the bounding box of the text in pixels
		 */
		const math::Rect& getBoundingBox() const;

	protected:
		/**
		 * Draws the text to the currently active render target.
		 * This is called by the render service when text is rendered with a user defined camera.
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix
		 */
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		/**
		 * Draws the text into to active render target using the provided matrices
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix
		 * @param modelMatrix the location of the text in the world
		 */
		void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix);

		FontInstance* mFont = nullptr;							///< Pointer to the font, set on initialization
		std::string mText = "";									///< Text to render
		MaterialInstance mMaterialInstance;						///< The MaterialInstance as created from the resource. 
		PlaneMesh mPlane;										///< Plane used to draws a single letter
		std::string mGlyphUniform = glyphUniform;				///< Name of the 2D texture character binding in the shader
		TransformComponentInstance* mTransform = nullptr;		///< Transform used to position text
		RenderableMesh mRenderableMesh;							///< Valid Plane / Material combination
		VertexAttribute<glm::vec3>* mPositionAttr = nullptr;	///< Handle to the plane vertices
		std::vector<RenderableGlyph*> mGlyphs;					///< Glyphs associated with the text to render
		math::Rect mTextBounds;									///< Bounds of the text in pixels
	};
}
