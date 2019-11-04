#pragma once

// External Includes
#include "texture2d.h"
#include "rendertarget.h"

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
		RenderTexture2D() = default;
		RenderTexture2D(RenderService& renderService);

		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int		mWidth  = 0;					///< Property: 'Width' width of the texture in texels
		int		mHeight = 0;					///< Property: 'Height' of the texture, in texels
		ERenderTargetFormat	mFormat = ERenderTargetFormat::RGB8;		///< Property: 'Format' format of the texture
	};

	using RenderTexture2DCreator = rtti::ObjectCreator<RenderTexture2D, RenderService>;
}
