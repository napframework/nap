#pragma once

#include "utility\dllexport.h"

namespace opengl
{
	struct Texture2DSettings;
}

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}
	class Bitmap;

	/**
	 */
	NAPAPI bool		getTextureSettingsFromBitmap(const Bitmap& bitmap, bool compress, opengl::Texture2DSettings& settings, nap::utility::ErrorState& errorState);
}