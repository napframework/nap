// Local Includes
#include "font.h"
#include "fontservice.h"

// External Includes
#include <nap/logger.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// nap::fontresource run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Font)
	RTTI_CONSTRUCTOR(nap::FontService&)
	RTTI_PROPERTY_FILELINK("Font",	&nap::Font::mFont,	nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Font)
	RTTI_PROPERTY("Size",			&nap::Font::mSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DPI",			&nap::Font::mDPI,	nap::rtti::EPropertyMetaData::Default)
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
		if (mFace != nullptr)
			FT_Done_Face(toFreetypeFace(mFace));
		mFreetypeLib = nullptr;
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
		nap::Logger::info("loading font: %s", mFont.c_str());

		// Get handle to freetype library
		mFreetypeLib = mService->getHandle();
		if (!errorState.check(mFreetypeLib != nullptr, "unable to acquire free-type library handle from service!"))
			return false;

		// Open default typeface
		FT_Face face;

		// Open first face
		auto fterror = FT_New_Face(toFreetypeLib(mFreetypeLib), mFont.c_str(), 0, &face);
		if (errorState.check(fterror == FT_Err_Unknown_File_Format, "unsupported font format"))
			return false;

		// Store handle
		mFace = face;

		// Other read error occurred
		if (errorState.check(fterror != 0, "invalid font, file could not be read or opened"))
			return false;

		// Make sure a default charmap is selected
		if (errorState.check(face->charmap == nullptr, "no default unicode charmap associated with typeface"))
			return false;

		// Set charachter size
		if (!errorState.check(setSize(mSize, mDPI), "unsupported font size and resolution"))
			return false;

		/*
		FT_Load_Glyph(face, 0, FT_LOAD_DEFAULT);
		FT_Bitmap bitmap = face->glyph->bitmap;
		FT_Pos height = height = face->glyph->metrics.height / 64;
		FT_Pos width = face->glyph->metrics.width / 64;
		*/

		return true;
	}


	bool Font::setSize(int size, int dpi)
	{
		assert(mFace != nullptr);
		if (FT_Set_Char_Size(toFreetypeFace(mFace), 0, mSize * 64, mDPI, mDPI) > 0)
			return false;
		mDPI  = dpi;
		mSize = size;
		return true;
	}


	void* Font::getFace() const
	{
		return mFace;
	}

}