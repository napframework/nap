#pragma once

// External Includes
#include "texture2d.h"

namespace nap
{
	/**
	 * GPU texture resource that it is initially empty
	 * This texture can be declared as a resource together with
	 * the format to use, width and height.
	 */
	class NAPAPI RenderTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		enum class EFormat
		{
			RGBA8,			///< 08 bit unsigned, 4 components
			RGB8,			///< 08 bit unsigned, 3 components
			R8,				///< 08 bit unsigned, 1 components
			RGBA16,			///< 16 bit unsigned, 4 components
			RGB16,			///< 16 bit unsigned, 3 components
			R16,			///< 16 bit unsigned, 1 components
			RGBA32,			///< 32 bit float, 4 components
			RGB32,			///< 32 bit float, 3 components
			R32,			///< 32 bit float, 1 components
			Depth			///< 32 bit float, depth buffer
		};

		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int		mWidth  = 0;					///< Property: 'Width' width of the texture in texels
		int		mHeight = 0;					///< Property: 'Height' of the texture, in texels
		EFormat	mFormat = EFormat::RGB8;		///< Property: 'Format' format of the texture
	};
}
