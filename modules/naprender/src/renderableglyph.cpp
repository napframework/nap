// Local Includes
#include "renderableglyph.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableGlyph)
RTTI_END_CLASS

namespace nap
{
	RenderableGlyph::RenderableGlyph(void* slot, uint index) : Glyph(slot, index)
	{ }

}