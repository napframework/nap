#pragma once

// External Includes
#include <glyph.h>
#include <texture2d.h>

namespace nap
{
	// Forward Declares
	class FontInstance;

	/**
	 * Represents a symbol (character) in a font that can be rendered using OpenGL
	 * The glyph is rendered to a 2D texture that can be tied to a material
	 * The stored glyph is converted to a bitmap on initialization and uploaded to the GPU
	 * The bitmap is deleted after storage on the GPU, the original Glyph remains intact. 
	 */
	class NAPAPI RenderableGlyph : public Glyph
	{
		friend FontInstance;
		RTTI_ENABLE(Glyph)

	public:
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
		const Texture2D& getTexture() const { return mTexture; }

		/**
		 * @return the 2D opengl Texture
		 */
		Texture2D& getTexture() { return mTexture; }

	protected:
		/**
		 * Constructor, can only be invoked by a FontInstance
		 * @param slot handle to the Glyph inside the font
		 * @param index index of the glyph in the font
		 */
		RenderableGlyph(void* handle, uint index);

		/**
		 * Initializes the OpenGL 2DTexture after construction of this glyph	
		 * First it converts the freetype glyph into a bitmap. This bitmap is uploaded to the GPU
		 * @return if the 2DTexture has been initialized correctly
		 */
		virtual bool init(utility::ErrorState& errorCode) override;

	private:
		Texture2D mTexture;
		glm::ivec2 mSize	= { -1, -1 };		///< Size of the Glyph in pixels
		glm::ivec2 mBearing = { -1, -1 };		///< Offset from baseline to left/top of glyph
	};
}
