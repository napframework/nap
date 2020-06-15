// Local Includes
#include "renderableglyph.h"
#include "renderservice.h"

// External Includes
#include <mathutils.h>
#include <ft2build.h>
#include <bitmap.h>
#include <nap/core.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableGlyph)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Renderable2DGlyph)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Renderable2DMipMapGlyph)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Converts a void pointer into a free type glyph object
	 */
	inline static FT_Glyph toFreeTypeGlyph(void* handle)
	{
		return reinterpret_cast<FT_Glyph>(handle);
	}


	RenderableGlyph::RenderableGlyph(nap::Core& core) : IGlyphRepresentation(core)
	{ }


	RenderableGlyph::~RenderableGlyph()
	{
		mTexture.reset(nullptr);
	}


	bool RenderableGlyph::onInit(const Glyph& glyph, utility::ErrorState& errorCode)
	{
		// Get handle to the glyph
		FT_Glyph bitmap = toFreeTypeGlyph(glyph.getHandle());

		// Convert to bitmap, this actually replaces the handle underneath
		// This means that the original pointer to the glyph is still valid!
		// We clear the bitmap data at the end of this function to safe memory
		FT_Vector  origin = {0, 0};
		auto error = FT_Glyph_To_Bitmap(&bitmap, FT_RENDER_MODE_NORMAL, &origin, false);
		if (!errorCode.check(error == 0, "unable to convert glyph to bitmap"))
			return false;
		
		// cast to bitmap and extract values
		FT_BitmapGlyph bitmap_glyph = reinterpret_cast<FT_BitmapGlyph>(bitmap);
		mBearing.x = bitmap_glyph->left;
		mBearing.y = bitmap_glyph->top;
		mSize.x = bitmap_glyph->bitmap.width;
		mSize.y = bitmap_glyph->bitmap.rows;

		// Store advance
		mAdvance.x = glyph.getHorizontalAdvance();
		mAdvance.y = glyph.getVerticalAdvance();

		// If there is no width / height, don't initialize the texture
		assert(mTexture == nullptr);
		if (mSize.x == 0 || mSize.y == 0)
		{
			FT_Done_Glyph(bitmap);
			return true;
		}

		// Create texture and get parameters
		mTexture = std::make_unique<Texture2D>(*mCore);

		// Initialize texture
		SurfaceDescriptor settings;
		settings.mWidth  = mSize.x;
		settings.mHeight = mSize.y;
		settings.mDataType = ESurfaceDataType::BYTE;
		settings.mChannels = ESurfaceChannels::R;
		
		if (!mTexture->init(settings, false, bitmap_glyph->bitmap.buffer, errorCode))
			return false;

		// Clean up bitmap data
		FT_Done_Glyph(bitmap);
		return true;
	}


	Renderable2DGlyph::Renderable2DGlyph(nap::Core& core) : RenderableGlyph(core)
	{
	}

	Renderable2DMipMapGlyph::Renderable2DMipMapGlyph(nap::Core& core) : RenderableGlyph(core)
	{
	}
}