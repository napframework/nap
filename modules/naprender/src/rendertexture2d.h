/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include "texture2d.h"
#include "irendertarget.h"

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 * Empty 2D GPU texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a nap::RenderTarget.
	 * When usage is 'Static' and 'Fill' is turned off the texture on the GPU is in an undefined state until being rendered to.
	 * This is ok when using the texture as a render target, before the texture is read somewhere else.
	 * All other usage modes initialize the texture to black.
	 */
	class NAPAPI RenderTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * All supported render texture 2D formats.
		 */
		enum class EFormat
		{
			RGBA8,			///< 08 bit unsigned, 4 components
			R8,				///< 08 bit unsigned, 1 component
			RGBA16,			///< 16 bit unsigned, 4 components
			R16,			///< 16 bit unsigned, 1 component
			RGBA32,			///< 32 bit float, 4 components
			R32				///< 32 bit float, 1 component
		};

		RenderTexture2D(Core& renderService);

		/**
		 * Creates the texture on the GPU.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int					mWidth		= 0;								///< Property: 'Width' width of the texture in texels
		int					mHeight		= 0;								///< Property: 'Height' of the texture in texels
		EColorSpace			mColorSpace	= EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EFormat				mFormat		= EFormat::RGBA8;					///< Property: 'Format' texture format
		bool				mFill		= false;							///< Property: 'Fill' if the texture is initialized to black when usage is static
	};
}
