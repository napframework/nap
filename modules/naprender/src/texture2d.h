#pragma once

// External Includes
#include "basetexture2d.h"

namespace nap
{
	/**
	 * GPU texture resource that it is initially empty
	 * This texture can be declared as a resource together with
	 * the format to use and a with and height.
	 */
	class NAPAPI Texture2D : public BaseTexture2D
	{
		RTTI_ENABLE(BaseTexture2D)
	public:
		enum class EFormat
		{
			RGBA8,			// 4 components, 8 bytes per component
			RGB8,			// 3 components, 8 bytes per component
			R8,				// 1 components, 8 bytes per component
			Depth			// Texture used for binding to depth buffer
		};

		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int		mWidth;			// Width of the texture, in texels
		int		mHeight;		// Height of the texture, in texels
		EFormat	mFormat;		// Format of the texture
	};
}
