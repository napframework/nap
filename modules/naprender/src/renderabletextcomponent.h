#pragma once

// Local Includes
#include "rendercomponent.h"
#include "materialinstance.h"
#include "renderableglyph.h"
#include "uniforminstance.h"

// External Includes
#include <font.h>
#include <planemesh.h>
#include <renderablemesh.h>
#include <transformcomponent.h>

namespace nap
{
	class RenderableTextComponentInstance;

	/**
	 * Render-able Text Component Resource.
	 * Creates a RenderableTextComponentInstance that can draw text using a font and material.
	 * Text rendering works best when the blend mode of the Material is set to: AlphaBlend and the Depth mode to NoReadWrite.
	 * This ensures the text is rendered on top of the rest and remains visible.
	 * Use the Renderable2DTextComponent to render text in screen space and the Renderable3DTextComopnent to render text in 3D space.
	 */
	class NAPAPI RenderableTextComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableTextComponent, RenderableTextComponentInstance)
	public:
		ResourcePtr<Font> mFont;								///< Property: 'Font' that represents the style of the text
		std::string mText;										///< Property: 'Text' to draw
		MaterialInstanceResource mMaterialInstanceResource;		///< Property: 'MaterialInstance' the material used to shade the text
		std::string mGlyphUniform = "glyph";					///< Property: 'GlyphUniform' name of the 2D texture character binding in the shader, defaults to 'glyph'
	};


	/**
	 * Draws text into the currently active render target using a font and material.
	 * This is the runtime version of the RenderableTextComponent resource.
	 * Text rendering works best when the blend mode of the Material is set to: AlphaBlend and the Depth mode to NoReadWrite. 
	 * This ensures the text is rendered on top of the rest and remains visible.
	 * Use the Renderable2DTextComponent to render text in screen space and the Renderable3DTextComopnent to render text in 3D space.
	 */
	class NAPAPI RenderableTextComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:

		RenderableTextComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initializes the this component.
		 * @param errorState holds the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

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
		const std::string& getText() const								{ return mText; }

		/**
		 * @return material used when drawing the text.
		 */
		MaterialInstance& getMaterialInstance();

		/**
		 * @return the bounding box of the text in pixels.
		 */
		const math::Rect& getBoundingBox() const;

		/**
		* Needs to be implemented by derived classes. Creates a RenderableGlyph for the given index in the font.
		* @param index the index to create the renderable glyph for.
		* @param error contains the error if the glyph representation could not be created.
		* @return the renderable glyph for the given character index.
		*/
		virtual RenderableGlyph* getRenderableGlyph(uint index, utility::ErrorState& error) const = 0;

	protected:
		/**
		 * Draws the text into to active render target using the provided matrices.
		 * Call this in derived classes based on extracted matrices.
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix
		 * @param modelMatrix the location of the text in the world
		 */
		void draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix);

		/**
		 * @return if this text has a transform component associated with it
		 */
		bool hasTransform() const										{ return mTransform != nullptr; }

		/**
		 * @return the transform component, nullptr if not initialized or not found
		 */
		const nap::TransformComponentInstance* getTransform() const		{ return mTransform; }			

		FontInstance* mFont = nullptr;									///< Pointer to the font, set on initialization
		RenderService* mRenderService = nullptr;						///< Pointer to the Renderer

	private:
		std::string mText = "";											///< Text to render
		MaterialInstance mMaterialInstance;								///< The MaterialInstance as created from the resource. 
		PlaneMesh mPlane;												///< Plane used to draws a single letter
		std::string mGlyphUniformName = "glyph";						///< Name of the 2D texture character binding in the shader
		Sampler2DInstance* mGlyphUniform = nullptr;						///< Found glyph uniform
		UniformMat4Instance* mModelUniform = nullptr;					///< Found model matrix uniform input
		UniformMat4Instance* mViewUniform = nullptr;					///< Found view matrix uniform input
		UniformMat4Instance* mProjectionUniform = nullptr;				///< Found projection uniform input
		TransformComponentInstance* mTransform = nullptr;				///< Transform used to position text
		RenderableMesh mRenderableMesh;									///< Valid Plane / Material combination
		VertexAttribute<glm::vec3>* mPositionAttr = nullptr;			///< Handle to the plane vertices
		std::vector<RenderableGlyph*> mGlyphs;							///< Glyphs associated with the text to render
		math::Rect mTextBounds;											///< Bounds of the text in pixels
	};
}
