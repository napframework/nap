#pragma once

// External Includes
#include "texture2d.h"
#include "irendertarget.h"

namespace nap
{
	class Core;

	enum class ERenderTargetFormat
	{
		RGBA8,			///< RGBA8 4 components, 8 bytes per component
		R8				///< R8	1 components, 8 bytes per component
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
		EColorSpace			mColorSpace	= EColorSpace::Linear;				///< Property: 'ColorSpace' colorspace of the texture
		ERenderTargetFormat	mFormat		= ERenderTargetFormat::RGBA8;		///< Property: 'Format' texture format
	};
}
