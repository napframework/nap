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
	NAPAPI bool		getTextureSettingsFromPixmap(const Bitmap& pixmap, bool compress, opengl::Texture2DSettings& settings, nap::utility::ErrorState& errorState);
}