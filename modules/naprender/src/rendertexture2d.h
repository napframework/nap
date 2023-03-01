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

	/**
	 * Empty 2D GPU texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a nap::RenderTarget or
	 * any other type of render operation. The texture is cleared to 'ClearColor the before first use.
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
			BGRA8,			///< 08 bit unsigned, 4 components
			R8,				///< 08 bit unsigned, 1 component
			RGBA16,			///< 16 bit unsigned, 4 components
			R16,			///< 16 bit unsigned, 1 component
			RGBA32,			///< 32 bit float, 4 components
			R32				///< 32 bit float, 1 component
		};

		RenderTexture2D(Core& core);

		/**
		 * Creates the texture on the GPU.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return Vulkan GPU data handle, including image and view.
		 */
		virtual const ImageData& getHandle() const override { return mImageData; }

		int					mWidth = 0;										///< Property: 'Width' width of the texture in texels
		int					mHeight = 0;									///< Property: 'Height' of the texture in texels
		EColorSpace			mColorSpace = EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EFormat				mColorFormat = EFormat::RGBA8;					///< Property: 'ColorFormat' color texture format
		RGBAColorFloat		mClearColor	= { 0.0f, 0.0f, 0.0f, 0.0f };		///< Property: 'ClearColor' color selection used for clearing the texture
	};


	/**
	 * Empty 2D GPU depth texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a nap::RenderTarget or
	 * any other type of render operation. The texture is cleared to 'ClearColor the before first use.
	 */
	class NAPAPI DepthRenderTexture2D : public Texture2D
	{
		friend class DepthRenderTarget;
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * All supported depth texture 2D formats.
		 */
		enum class EDepthFormat
		{
			D16,			///< 16 bit unsigned, 1 component
			D32				///< 32 bit float, 1 component
		};

		DepthRenderTexture2D(Core& core);

		/**
		 * Creates the texture on the GPU.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int					mWidth = 0;										///< Property: 'Width' width of the texture in texels
		int					mHeight = 0;									///< Property: 'Height' of the texture in texels
		EColorSpace			mColorSpace = EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EDepthFormat		mDepthFormat = EDepthFormat::D16;				///< Property: 'DepthFormat' depth texture format
		float				mClearValue = 1.0f;								///< Property: 'ClearValue' value selection used for clearing the texture
		bool				mFill = false;									///< Property: 'Fill' if the texture is initialized to black when usage is static
	};


	/**
	 * Empty 2D GPU texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a nap::RenderTarget or
	 * any other type of render operation. The texture is cleared to 'ClearColor the before first use.
	 */
	class NAPAPI RenderTextureCube : public TextureCube
	{
		RTTI_ENABLE(TextureCube)
	public:
		/**
		 * All supported render texture 2D formats.
		 */
		enum class EFormat
		{
			RGBA8,			///< 08 bit unsigned, 4 components
			BGRA8,			///< 08 bit unsigned, 4 components
			R8,				///< 08 bit unsigned, 1 component
			RGBA16,			///< 16 bit unsigned, 4 components
			R16,			///< 16 bit unsigned, 1 component
			RGBA32,			///< 32 bit float, 4 components
			R32				///< 32 bit float, 1 component
		};

		RenderTextureCube(Core& core);

		/**
		 * Creates the texture on the GPU.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int					mWidth = 0;										///< Property: 'Width' width of the texture in texels
		int					mHeight = 0;									///< Property: 'Height' of the texture in texels
		EColorSpace			mColorSpace = EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EFormat				mColorFormat = EFormat::RGBA8;					///< Property: 'ColorFormat' color texture format
		RGBAColorFloat		mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };		///< Property: 'ClearColor' color selection used for clearing the texture
	};
}
