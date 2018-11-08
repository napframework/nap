#include "glyph.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// RTTI Glyph Definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Glyph)
RTTI_END_CLASS


namespace nap
{
	/**
	 * Constructor
	 */
	Glyph::Glyph(void* slot, uint index) : mHandle(slot),
		mIndex(index)  
	{
		mAdvance.x = reinterpret_cast<FT_Glyph>(mHandle)->advance.x >> 16;
		mAdvance.y = reinterpret_cast<FT_Glyph>(mHandle)->advance.y >> 16;
	}


	/**
	 * Destructor
	 */
	Glyph::~Glyph()
	{
		assert(mHandle != nullptr);
		FT_Done_Glyph(reinterpret_cast<FT_Glyph>(mHandle));
	}


	bool Glyph::isValid()
	{
		return mHandle != nullptr;
	}
}