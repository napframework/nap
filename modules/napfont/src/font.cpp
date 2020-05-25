// Local Includes
#include "font.h"
#include "fontservice.h"

// External Includes
#include <nap/logger.h>
#include <nap/core.h>
#include <mathutils.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

RTTI_BEGIN_STRUCT(nap::FontProperties)
	RTTI_VALUE_CONSTRUCTOR(int, int)
	RTTI_PROPERTY("Size",		&nap::FontProperties::mSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DPI",		&nap::FontProperties::mDPI,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::fontresource run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Font)
	RTTI_CONSTRUCTOR(nap::FontService&)
	RTTI_PROPERTY("Properties",		&nap::Font::mProperties,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY_FILELINK("Font",	&nap::Font::mFont,			nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Font)
RTTI_END_CLASS

// nap::fontinstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FontInstance)
	RTTI_CONSTRUCTOR(nap::FontService&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

/**
 * Converts a void pointer into a free type library object
 */
inline static FT_Library toFreetypeLib(void* handle)
{
	return reinterpret_cast<FT_Library>(handle);
}


/**
 *	Converts a void pointer into a free type face object
 */
inline static FT_Face toFreetypeFace(void* handle)
{
	return reinterpret_cast<FT_Face>(handle);
}


namespace nap
{
	Font::~Font()			
	{
		// Reset to null, de-allocating the instance
		mInstance.reset(nullptr);
		mService = nullptr;
	}


	Font::Font(FontService& service)
	{
		mService = &service;
	}


	bool Font::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mService != nullptr, "unable to acquire handle to font service"))
			return false;
		
		// Create font instance
		mInstance = std::make_unique<FontInstance>(*mService);

		// Load typeface
		nap::Logger::info("loading font: %s", mFont.c_str());
		if (!mInstance->create(mFont, mProperties, errorState))
			return false;

		return true;
	}


	nap::FontInstance& Font::getFontInstance()
	{
		assert(mInstance != nullptr);
		return *mInstance;
	}


	const nap::FontInstance& Font::getFontInstance() const
	{
		assert(mInstance != nullptr);
		return *mInstance;
	}


	FontInstance::FontInstance(FontService& service) : mService(&service)
	{
		// Get handle to free-type lib
		mFreetypeLib = service.mFreetypeLib;
	}


	void* FontInstance::getFace() const
	{
		return mFace;
	}


	bool FontInstance::create(const std::string& font, FontProperties& properties, utility::ErrorState& error)
	{
		if (!error.check(mFreetypeLib != nullptr, "unable to acquire free-type library handle from service!"))
			return false;

		// Open default typeface
		FT_Face face;

		// Open first face
		auto fterror = FT_New_Face(toFreetypeLib(mFreetypeLib), font.c_str(), 0, &face);
		if (!error.check(fterror != FT_Err_Unknown_File_Format, "unsupported font format"))
			return false;

		// Other read error occurred
		if (!error.check(fterror == 0, "invalid font, file could not be read or opened"))
			return false;

		// Make sure a default charmap is selected
		if (!error.check(face->charmap != nullptr, "no default unicode charmap associated with typeface"))
			return false;

		// Set character size
		fterror = FT_Set_Char_Size(face, 0, properties.mSize * 64, properties.mDPI, properties.mDPI);
		if (!error.check(fterror == 0, "unsupported font size and resolution"))
			return false;

		// Remove previous typeface if present
		if (mFace != nullptr)
		{
			fterror = FT_Done_Face(toFreetypeFace(mFace));
			if (fterror > 0)
				nap::Logger::warn("unable to de-allocate type face: %s", font.c_str());
		}

		// Store handle
		mFace = face;

		// Store properties and clear cache on next fetch
		mProperties = properties;
		mFont = font;
		resetCache();

		return true;
	}


	bool FontInstance::isValid() const
	{
		return mFace != nullptr;
	}


	FontInstance::~FontInstance()
	{
		// Remove all cached glyphs
		mGlyphs.clear();

		// Free type face
		if(isValid())
			FT_Done_Face(toFreetypeFace(mFace));
		mFace = nullptr;
		mFreetypeLib = nullptr;
	}


	const FontProperties& FontInstance::getProperties() const
	{
		return mProperties;
	}


	nap::uint FontInstance::getGlyphIndex(nap::uint charCode) const
	{
		assert(isValid());
		return FT_Get_Char_Index(toFreetypeFace(mFace), static_cast<FT_ULong>(charCode));
	}


	bool FontInstance::loadGlyph(uint index)
	{
		if (FT_Load_Glyph(toFreetypeFace(mFace), index, FT_LOAD_DEFAULT) > 0)
			return false;
		return true;
	}


	void* nap::FontInstance::getGlyphHandle()
	{
		FT_Glyph  new_glyph;
		if (FT_Get_Glyph(toFreetypeFace(mFace)->glyph, &new_glyph) > 0)
			return nullptr;
		return new_glyph;
	}


	void FontInstance::resetCache()
	{
		assert(isValid());
		mGlyphs.clear();
		mGlyphs.resize(toFreetypeFace(mFace)->num_glyphs);
	}


	int FontInstance::getCount()
	{
		return mFace != nullptr ? toFreetypeFace(mFace)->num_glyphs : -1;
	}



	const std::string& FontInstance::getFont() const
	{
		return mFont;
	}


	GlyphCache* nap::FontInstance::getOrCreateGlyphCache(nap::uint index, utility::ErrorState& errorCode)
	{
		// Ensure the index is valid
		assert(isValid());
		if (!errorCode.check(index < mGlyphs.size(), "glyph index out of bounds"))
			return nullptr;
		
		// Try to find a cached Glyph
		std::unique_ptr<GlyphCache>& cache = mGlyphs[index];

		// Parent glyph is part of system
		if (cache != nullptr)
			return cache.get();

		// Load a new glyph
		if (!errorCode.check(loadGlyph(index), "unable to load glyph: %d into memory", index))
			return nullptr;

		// Copy handle
		void* glyph_handle = getGlyphHandle();
		if (!errorCode.check(glyph_handle != nullptr, "unable to get handle to glyph: %d", index))
			return nullptr;

		// Create new glyph, constructor is protected so new is required
		Glyph* ptr = new Glyph(glyph_handle, index);

		// Now wrap a Glyph cache around it so we can bind it to various representations
		GlyphCache* new_cache = new GlyphCache(std::unique_ptr<Glyph>(ptr));
		cache.reset(new_cache);
		return new_cache;
	}


	void FontInstance::getBoundingBox(const std::string& text, math::Rect& outRect)
	{
		// Clear if text is empty
		if (text.empty())
		{
			outRect.mMinPosition = { 0.0f, 0.0f };
			outRect.mMaxPosition = { 0.0f, 0.0f };
			return;
		}

		// Compute bounds
		FT_BBox ft_box;
		outRect.mMinPosition = { 0.0f, math::max<float>() };
		outRect.mMaxPosition = { 0.0f, math::min<float>() };
		utility::ErrorState error;
		int idx = 0;
		for (const auto& letter : text)
		{
			const Glyph* glyph = getOrCreateGlyph(getGlyphIndex(letter), error);
			if (glyph == nullptr)
			{
				nap::Logger::warn(error.toString());
				continue;
			}

			// Get glyph and glyph bounds
			FT_Glyph ft_glyph = reinterpret_cast<FT_Glyph>(glyph->getHandle());
			FT_Glyph_Get_CBox(ft_glyph, FT_GLYPH_BBOX_TRUNCATE, &ft_box);
			
			// If it's the first character we want to get the x-offset
			if (idx == 0)
			{
				outRect.mMinPosition.x = static_cast<float>(ft_box.xMin);
				outRect.mMaxPosition.x = static_cast<float>(ft_box.xMin);
			}
			// Advance or, when last character, pick width
			if (idx == (text.size() - 1))
			{
				outRect.mMaxPosition.x += static_cast<float>(ft_box.xMax - ft_box.xMin);
			}
			else
			{
				outRect.mMaxPosition.x += static_cast<float>(glyph->getHorizontalAdvance());
			}
			// Store y min using bound comparison
			if (static_cast<float>(ft_box.yMin) < outRect.mMinPosition.y)
			{
				outRect.mMinPosition.y = static_cast<float>(ft_box.yMin);
			}
			// Store y max using bound comparison
			if (static_cast<float>(ft_box.yMax) > outRect.mMaxPosition.y)
			{
				outRect.mMaxPosition.y = static_cast<float>(ft_box.yMax);
			}
			idx++;
		}
	}


	IGlyphRepresentation* FontInstance::getOrCreateGlyphRepresentation(nap::uint index, const rtti::TypeInfo& type, utility::ErrorState& errorCode)
	{
		// Acquire handle to glyph cache
		assert(isValid());
		GlyphCache* cache = getOrCreateGlyphCache(index, errorCode);
		if (cache == nullptr)
			return nullptr;

		// Find requested representation of this glyph
		nap::IGlyphRepresentation* representation = cache->findRepresentation(type);
		if (representation != nullptr)
			return representation;

		// Add new representation and move to unique ptr
		IGlyphRepresentation* new_rep = type.create<IGlyphRepresentation>({ mService->getCore() });
		if (!errorCode.check(new_rep != nullptr, ":%s is not a valid IGlyphRepresentation object", type.get_name().to_string().c_str()))
			return nullptr;

		// Wrap and initialize
		std::unique_ptr<IGlyphRepresentation> urep(new_rep);
		if (!urep->init(cache->getGlyph(), errorCode))
			return nullptr;

		// Add to cache
		cache->addRepresentation(std::move(urep));
		return new_rep;
	}


	//////////////////////////////////////////////////////////////////////////
	// GlyphCache
	//////////////////////////////////////////////////////////////////////////

	GlyphCache::~GlyphCache()
	{
		mRepresentations.clear();
		mGlyph.reset(nullptr);
	}


	void GlyphCache::addRepresentation(std::unique_ptr<IGlyphRepresentation> representation)
	{
		mRepresentations.emplace(std::make_pair(representation->get_type().get_raw_type(), std::move(representation)));
	}


	nap::IGlyphRepresentation* GlyphCache::findRepresentation(const rtti::TypeInfo& type)
	{
		auto it = mRepresentations.find(type.get_raw_type());
		return it != mRepresentations.end() ? it->second.get() : nullptr;
	}


	const Glyph* nap::FontInstance::getOrCreateGlyph(nap::uint index, utility::ErrorState& errorCode)
	{
		nap::GlyphCache* cache = getOrCreateGlyphCache(index, errorCode);
		if (cache == nullptr)
			return nullptr;
		return &(cache->getGlyph());
	}
}