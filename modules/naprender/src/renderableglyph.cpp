// Local Includes
#include "renderableglyph.h"

// External Includes
#include <mathutils.h>
#include <ft2build.h>
#include <bitmap.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableGlyph)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Renderable2DGlyph)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Renderable2DMipMapGlyph)
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

		// Set parameters
		getTextureParameters(mTexture.mParameters, mSize);

		// Initialize texture
		Texture2DSettings settings;
		settings.mWidth  = mSize.x;
		settings.mHeight = mSize.y;
		settings.mDataType = ESurfaceDataType::BYTE;
		settings.mChannels = ESurfaceChannels::R;
		if (!mTexture.initTexture(settings, errorCode))
			return false;

		// Upload glyph bitmap data
		//mTexture.update(bitmap_glyph->bitmap.buffer, bitmap_glyph->bitmap.pitch);

		// Clean up bitmap data
		FT_Done_Glyph(bitmap);

		// Store advance
		mAdvance.x = glyph.getHorizontalAdvance();
		mAdvance.y = glyph.getVerticalAdvance();

		return true;
	}


	void Renderable2DGlyph::getTextureParameters(TextureParameters& outParameters, const glm::ivec2& charSize)
	{
		outParameters.mMaxFilter		= EFilterMode::Linear;
		outParameters.mMinFilter		= EFilterMode::Linear;
		outParameters.mMaxLodLevel		= 0;
		outParameters.mWrapVertical		= EWrapMode::ClampToEdge;
		outParameters.mWrapVertical		= EWrapMode::ClampToEdge;
	}


	void Renderable2DMipMapGlyph::getTextureParameters(TextureParameters& outParameters, const glm::ivec2& charSize)
	{
		// Calculate max character lod value based on character dimensions
		int max_lod = charSize.x > charSize.y ? charSize.x : charSize.y;
		max_lod = int(glm::log2((float)std::max<int>(max_lod,1)));

		outParameters.mMaxFilter		= EFilterMode::Linear;
		outParameters.mMinFilter		= max_lod > 0 ? EFilterMode::LinearMipmapLinear : EFilterMode::Linear;
		outParameters.mMaxLodLevel		= max_lod;
		outParameters.mWrapVertical		= EWrapMode::ClampToEdge;
		outParameters.mWrapHorizontal	= EWrapMode::ClampToEdge;
	}

}