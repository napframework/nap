// Local Includes
#include "font.h"
#include "fontservice.h"

// External Includes
#include <nap/logger.h>
#include <ft2build.h>
#include FT_FREETYPE_H

RTTI_BEGIN_STRUCT(nap::FontProperties)
	RTTI_VALUE_CONSTRUCTOR(int, int, const std::string&)
	RTTI_PROPERTY("Font",	&nap::FontProperties::mFont,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Size",	&nap::FontProperties::mSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DPI",	&nap::FontProperties::mDPI,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

// nap::fontresource run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Font)
	RTTI_CONSTRUCTOR(nap::FontService&)
	RTTI_PROPERTY("Properties", &nap::Font::mProperties, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::fontinstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FontInstance)
	RTTI_CONSTRUCTOR(const nap::FontService&)
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
		mInstance.reset();
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
		nap::Logger::info("loading font: %s", mProperties.mFont.c_str());
		if (!mInstance->create(mProperties, errorState))
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


	FontInstance::FontInstance(const FontService& service)
	{
		// Get handle to freetype lib
		mFreetypeLib = service.mFreetypeLib;
	}


	void* FontInstance::getFace() const
	{
		return mFace;
	}


	bool FontInstance::create(const FontProperties& properties, utility::ErrorState& error)
	{
		if (!error.check(mFreetypeLib != nullptr, "unable to acquire free-type library handle from service!"))
			return false;

		// Open default typeface
		FT_Face face;

		// Open first face
		auto fterror = FT_New_Face(toFreetypeLib(mFreetypeLib), properties.mFont.c_str(), 0, &face);
		if (error.check(fterror == FT_Err_Unknown_File_Format, "unsupported font format"))
			return false;

		// Other read error occurred
		if (error.check(fterror != 0, "invalid font, file could not be read or opened"))
			return false;

		// Make sure a default charmap is selected
		if (error.check(face->charmap == nullptr, "no default unicode charmap associated with typeface"))
			return false;

		// Set character size
		fterror = FT_Set_Char_Size(face, 0, properties.mSize * 64, properties.mDPI, properties.mDPI);
		if (error.check(fterror > 0, "unsupported font size and resolution"))
			return false;

		// Remove previous typeface if present
		if (mFace != nullptr)
		{
			fterror = FT_Done_Face(toFreetypeFace(mFace));
			if (fterror > 0)
				nap::Logger::warn("unable to de-allocate type face: %s", properties.mFont.c_str());
		}

		// Store handle
		mFace = face;

		// Store properties
		mProperties = properties;

		/*
		FT_Load_Glyph(face, 0, FT_LOAD_DEFAULT);
		FT_Bitmap bitmap = face->glyph->bitmap;
		FT_Pos height = height = face->glyph->metrics.height / 64;
		FT_Pos width = face->glyph->metrics.width / 64;
		*/

		return true;
	}


	bool FontInstance::changeSize(int size, int dpi)
	{
		// Ensure we have a valid face
		assert(isValid());
		if (FT_Set_Char_Size(toFreetypeFace(mFace), 0, size * 64, dpi, dpi) > 0)
			return false;

		// Copy settings
		mProperties.mSize = size;
		mProperties.mDPI  = dpi;

		return true;
	}


	bool FontInstance::isValid() const
	{
		return mFace != nullptr;
	}


	FontInstance::~FontInstance()
	{
		mFreetypeLib = nullptr;
		if (mFace != nullptr)
			FT_Done_Face(toFreetypeFace(mFace));
		mFace = nullptr;
	}


	const FontProperties& FontInstance::getProperties() const
	{
		return mProperties;
	}

}