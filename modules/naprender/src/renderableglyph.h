#pragma once

// External Includes
#include <glyph.h>

namespace nap
{
	// Forward Declares
	class FontInstance;

	/**
	 * Represents a symbol (character) in a font that can be rendered using OpenGL
	 * The glyph is rendered to a 2D texture that is managed by this class
	 */
	class NAPAPI RenderableGlyph : public Glyph
	{
		friend FontInstance;
		RTTI_ENABLE(Glyph)

	protected:
		/**
		 * Constructor, can only be invoked by a FontInstance
		 * @param slot handle to the Glyph inside the font
		 * @param index index of the glyph in the font
		 */
		RenderableGlyph(void* slot, uint index);
	};
}
