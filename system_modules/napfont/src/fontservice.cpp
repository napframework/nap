/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "fontservice.h"
#include "font.h"

// External Includes
#include <ft2build.h>
#include FT_FREETYPE_H

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FontService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	FontService::FontService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	FontService::~FontService()
	{
		if (mFreetypeLib != nullptr)
			FT_Done_FreeType(reinterpret_cast<FT_Library>(mFreetypeLib));
	}


	bool FontService::init(utility::ErrorState& error)
	{
		// Init library
		FT_Library lib;
		if (!error.check(FT_Init_FreeType(&lib) == 0, "Unable to initialize FreeType Library"))
			return false;
		
		// Copy over handle
		mFreetypeLib = lib;
		return true;
	}


	void FontService::shutdown()
	{

	}


	void FontService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<FontObjectCreator>(*this));
	}


	void* FontService::getHandle() const
	{
		assert(mFreetypeLib != nullptr);
		return mFreetypeLib;
	}

}