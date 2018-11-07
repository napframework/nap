#include "glyph.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// RTTI Glyph Definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Glyph)
RTTI_END_CLASS

/**
 * Converts a void pointer into a free type glyph object
 */
inline static FT_Glyph toFreeTypeGlyph(void* handle)
{
	return reinterpret_cast<FT_Glyph>(handle);
}


namespace nap
{
	/**
	 * Constructor
	 */
	Glyph::Glyph(void* slot, uint index) : mSlot(slot),
		mIndex(index)  { }


	/**
	 * Destructor
	 */
	Glyph::~Glyph()
	{
		assert(mSlot != nullptr);
		FT_Done_Glyph(toFreeTypeGlyph(mSlot));
	}


	bool Glyph::isValid()
	{
		return mSlot != nullptr;
	}
}