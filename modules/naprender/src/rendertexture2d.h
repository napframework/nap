#pragma once

// External Includes
#include "texture2d.h"
#include "irendertarget.h"

namespace nap
{
	class Core;

	enum class ERenderTargetFormat
	{
		Backbuffer,		///< The current native format of the color render target, sample count based on supported msaa samples
		Depth			///< Depth Texture used for binding to depth buffer, sample count based supported on supported msaa samples
	};

	/**
	 * GPU texture resource that it is initially empty
	 * This texture can be declared as a resource together with
	 * the format to use, width and height.
	 */
	class NAPAPI RenderTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		RenderTexture2D(Core& renderService);

		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int					mWidth		= 0;								///< Property: 'Width' width of the texture in texels
		int					mHeight		= 0;								///< Property: 'Height' of the texture, in texels
		ERenderTargetFormat	mFormat		= ERenderTargetFormat::Backbuffer;	///< Property: 'Format' format of the texture
		EColorSpace			mColorSpace	= EColorSpace::Linear;				///< Property: 'ColorSpace' colorspace of the texture
	};
}
