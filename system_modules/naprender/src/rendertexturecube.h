/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include "texture.h"
#include "irendertarget.h"
#include <color.h>

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 * Empty cube GPU texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a `nap::CubeRenderTarget` or
	 * any other type of render operation. The texture is cleared to 'ClearColor' before first use.
	 *
	 * Internally, a cube texture creates a six-layer Vulkan image, each layer representing one side
	 * of a unit cube. Cube textures can be set to `nap::SamplerCubeInstance` for shader samplers of
	 * types `samplerCube` and `samplerCubeShadow`.
	 * 
	 * Cube image layers are addressed and oriented as follows:
	 * - Layer 0: right (+X)
	 * - Layer 1: left (-X)
	 * - Layer 2: up (+Y)
	 * - Layer 3: down (-Y)
	 * - Layer 4: back (+Z)
	 * - Layer 5: forward (-Z)
	 */
	class NAPAPI RenderTextureCube : public TextureCube
	{
		RTTI_ENABLE(TextureCube)
	public:
		/**
		 * All supported render texture cube formats.
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

		int					mWidth = 0;										///< Property: 'Width' width of a cube face texture in texels
		int					mHeight = 0;									///< Property: 'Height' of a cube face texture in texels
		EColorSpace			mColorSpace = EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EFormat				mColorFormat = EFormat::RGBA8;					///< Property: 'ColorFormat' color texture format
		RGBAColorFloat		mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };		///< Property: 'ClearColor' color selection used for clearing the texture

	protected:
		bool				mGenerateLODs = false;
	};


	/**
	 * Empty cube GPU depth texture that can be declared as a resource in JSON or created at runtime.
	 * You can use this texture to store the result of a render pass by a `nap::CubeDepthRenderTarget` or
	 * any other type of render operation. The texture is cleared to 'ClearValue' before first use.
	 *
	 * Internally, a cube texture creates a six-layer Vulkan image, each layer representing one side
	 * of a unit cube. Cube textures can be set to `nap::SamplerCubeInstance` for shader samplers of
	 * types `samplerCube` and `samplerCubeShadow`.
	 *
	 * Cube image layers are addressed and oriented as follows:
	 * - Layer 0: right (+X)
	 * - Layer 1: left (-X)
	 * - Layer 2: up (+Y)
	 * - Layer 3: down (-Y)
	 * - Layer 4: back (+Z)
	 * - Layer 5: forward (-Z)
	 */
	class NAPAPI DepthRenderTextureCube : public TextureCube
	{
		friend class CubeDepthRenderTarget;
		RTTI_ENABLE(TextureCube)
	public:
		/**
		 * All supported depth texture cube formats.
		 */
		enum class EDepthFormat
		{
			D16,			///< 16 bit unsigned, 1 component
			D32				///< 32 bit float, 1 component
		};

		DepthRenderTextureCube(Core& core);

		/**
		 * Creates the texture on the GPU.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int					mWidth = 0;										///< Property: 'Width' width of a cube face texture in texels
		int					mHeight = 0;									///< Property: 'Height' of a cube face texture in texels
		EColorSpace			mColorSpace = EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EDepthFormat		mDepthFormat = EDepthFormat::D16;				///< Property: 'DepthFormat' depth texture format
		float				mClearValue = 1.0f;								///< Property: 'ClearValue' value selection used for clearing the texture
		bool				mFill = false;									///< Property: 'Fill' if the texture is initialized to black when usage is static
	};
}
