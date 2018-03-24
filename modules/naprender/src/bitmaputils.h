#pragma once

#include "utility/dllexport.h"

namespace opengl
{
	struct Texture2DSettings;
}

namespace nap
{	
	// Forward Declares
	namespace utility
	{
		class ErrorState;
	}
	class Bitmap;

	/**
	 * Creates matching opengl texture settings based on the properties of a bitmap
	 * @param bitmap the bitmap used to construct a set of opengl texture settings
	 * @param compress if the opengl texture settings include a compressed format
	 * @param settings the constructed opengl texture settings based on the bitmap
	 * @param errorState contains the error when no settings could be derived from the bitmap
	 * @return if the opengl texture settings could be constructed successfully
	 */
	NAPAPI bool	getTextureSettingsFromBitmap(const Bitmap& bitmap, bool compress, opengl::Texture2DSettings& settings, nap::utility::ErrorState& errorState);
}