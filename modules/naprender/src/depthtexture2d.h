/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include "texture2d.h"
#include "irendertarget.h"
#include <color.h>

namespace nap
{
	// Forward Declares
	class Core;
	class DepthRenderTarget;

	/**
	 * Empty 2D GPU texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a nap::RenderTarget.
	 * When usage is 'Static' and 'Fill' is turned off the texture on the GPU is in an undefined state until being rendered to.
	 * This is ok when using the texture as a render target, before the texture is read somewhere else.
	 * All other usage modes initialize the texture to the specified clear color.
	 */
	class NAPAPI DepthTexture2D : public Texture2D
	{
		friend class DepthRenderTarget;
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * All supported render texture 2D formats.
		 */
		enum class EDepthFormat
		{
			D8,				///< 08 bit unsigned, 1 component
			D16,			///< 16 bit unsigned, 1 component
			D32				///< 32 bit float, 1 component
		};

		DepthTexture2D(Core& core);

		/**
		 * Creates the texture on the GPU.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return Vulkan image layout
		 */
		virtual VkImageLayout getImageLayout() const override { return mLayout; }


		int					mWidth		= 0;								///< Property: 'Width' width of the texture in texels
		int					mHeight		= 0;								///< Property: 'Height' of the texture in texels
		EColorSpace			mColorSpace	= EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EDepthFormat		mFormat		= EDepthFormat::D16;				///< Property: 'Format' texture format
		float				mClearValue = 1.0f;								///< Property: 'ClearValue' value selection used for clearing the texture
		bool				mFill		= false;							///< Property: 'Fill' if the texture is initialized to black when usage is static

	private:
		VkImageLayout		mLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	};
}
