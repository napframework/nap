// Local Includes
#include "font.h"
#include "fontservice.h"

// External Includes
#include <nap/logger.h>
#include <ft2build.h>
#include FT_FREETYPE_H

// nap::fontresource run time class definition 
RTTI_BEGIN_CLASS(nap::Font)
	RTTI_PROPERTY_FILELINK("Font", &nap::Font::mFont, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Font)
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
	}


	Font::Font(FontService& service)
	{
		mFreetypeLib = service.getHandle();
	}


	bool Font::init(utility::ErrorState& errorState)
	{
		nap::Logger::info("loading font: %s", mFont.c_str());

		if (!errorState.check(mFreetypeLib != nullptr, "unable to acquire free-type library handle from service!"))
			return false;

		// Open default typeface
		FT_Face face;

		// Open first face
		auto fterror = FT_New_Face(toFreetypeLib(mFreetypeLib), mFont.c_str(), 0, &face);
		if (errorState.check(fterror == FT_Err_Unknown_File_Format, "unsupported font format"))
			return false;

		// Other read error occurred
		if (errorState.check(fterror != 0, "invalid font, file could not be read or opened"))
			return false;

		// Store handle
		mFace = face;
		return true;
	}
}