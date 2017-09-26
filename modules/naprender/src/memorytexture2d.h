#pragma once

// External Includes
#include "texture2d.h"

namespace nap
{
	/**
	 * Memory texture that it is initially empty, that can be set from RTTI.
	 */
	class NAPAPI MemoryTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		enum class EFormat
		{
			RGBA8,			// 4 components, 8 bytes per component
			RGB8,			// 3 components, 8 bytes per component
			R8,				// 1 components, 8 bytes per component
			Depth			// Texture used for binding to depth buffer
		};

		int		mWidth;			// Width of the texture, in texels
		int		mHeight;		// Height of the texture, in texels
		EFormat	mFormat;		// Format of the texture
	};
}
