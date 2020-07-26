#pragma once

// External Includes
#include <glyph.h>
#include <texture2d.h>

namespace nap
{
	// Forward Declares
	class FontInstance;
	class RenderService;
	class Core;

	/**
	 * Represents a symbol (character) in a font that can be rendered.
	 * The glyph is rendered to a 2D texture that can be tied to a material.
	 * The glyph is converted to a bitmap on initialization and uploaded to the GPU.
	 * The bitmap is deleted after storage on the GPU.
	 */
	class NAPAPI RenderableGlyph : public IGlyphRepresentation
	{
		RTTI_ENABLE(IGlyphRepresentation)
	public:
		// Constructor
		RenderableGlyph(nap::Core& core);

		// Destructor
		virtual ~RenderableGlyph() override;

		/**
		 * @return size of the glyph in pixels
		 */
		const glm::ivec2& getSize() const { return mSize; }

		/**
		 * @return offset from baseline to top / left of glyph in pixels
		 */
		const glm::ivec2& getBearing() const { return mBearing; }

		/**
		 * @return width of the glyph in pixels
		 */
		int getWidth() const { return mSize.x; }

		/**
		 * @return height of the glyph in pixels
		 */
		int getHeight() const { return mSize.y; }

		/**
		 * @return Offset from baseline to the left in pixels
		 */
		int getOffsetLeft() const { return mBearing.x; }

		/**
		 * @return Offset from baseline to the left in pixels
		 */
		int getOffsetTop() const { return mBearing.y; }

		/**
		 * @return the 2D opengl Texture
		 */
		const Texture2D& getTexture() const { return *mTexture; }

		/**
		 * @return the 2D opengl Texture
		 */
		Texture2D& getTexture() { return *mTexture; }

		/**
		 * @return horizontal glyph advance value in pixels
		 */
		int getHorizontalAdvance() const { return mAdvance.x; }

		/**
		 * @return vertical glyph advance value in pixels
		 */
		int getVerticalAdvance() const { return mAdvance.y; }

		/**
		 * If this render-able glyph has a texture. 
		 * @return if this glyph has a texture associated with it
		 */
		bool empty() const { return mTexture == nullptr; }

	protected:
		/**
		 * Initializes the OpenGL 2DTexture after construction of this glyph.	
		 * First it converts the free-type glyph into a bitmap. This bitmap is uploaded to the GPU.
		 * @return if the 2DTexture has been initialized correctly.
		 */
		virtual bool setup(const Glyph& glyph, bool generateMipmaps, utility::ErrorState& errorCode);

	private:
		std::unique_ptr<Texture2D> mTexture = nullptr;
		glm::ivec2 mSize	= { -1, -1 };		///< Size of the Glyph in pixels
		glm::ivec2 mBearing = { -1, -1 };		///< Offset from baseline to left/top of glyph
		glm::ivec2 mAdvance = { -1, -1 };		///< Offset in pixels to advance to next glyph
	};


	//////////////////////////////////////////////////////////////////////////
	// Renderable2DGlyph
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a symbol (character) in a font that can be rendered.
	 * The glyph is rendered to a 2DTexture and has no mipmaps.
	 * Use this glyph representation when the text does not scale.
	 */
	class NAPAPI Renderable2DGlyph : public RenderableGlyph
	{
		RTTI_ENABLE(RenderableGlyph)
	public:
		// Constructor
		Renderable2DGlyph(nap::Core& core);

	protected:
		/**
		 * Initializes the OpenGL 2DTexture after construction of this glyph.
		 * First it converts the free-type glyph into a bitmap. This bitmap is uploaded to the GPU.
		 * @return if the 2DTexture has been initialized correctly.
		 */
		virtual bool onInit(const Glyph& glyph, utility::ErrorState& errorCode) override;
	};


	//////////////////////////////////////////////////////////////////////////
	// Renderable2DMipMapGlyph
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a symbol (character) in a font that can be rendered.
	 * The glyph is rendered to a 2DTexture but has mipmaps.
	 * Use this glyph when the text is not rendered in the native font resolution (ie: scaled).
	 */
	class NAPAPI Renderable2DMipMapGlyph : public RenderableGlyph
	{
		RTTI_ENABLE(RenderableGlyph)
	public:
		// Constructor
		Renderable2DMipMapGlyph(nap::Core& core);

	protected:
		/**
		 * Initializes the OpenGL 2DTexture after construction of this glyph.
		 * First it converts the free-type glyph into a bitmap. This bitmap is uploaded to the GPU.
		 * @return if the 2DTexture has been initialized correctly.
		 */
		virtual bool onInit(const Glyph& glyph, utility::ErrorState& errorCode) override;
	};
}
