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
	class Pixmap;

	/**
	 */
	NAPAPI bool		getTextureSettingsFromPixmap(const Pixmap& pixmap, bool compress, opengl::Texture2DSettings& settings, nap::utility::ErrorState& errorState);
}