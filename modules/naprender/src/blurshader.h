/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <shader.h>

namespace nap
{
	// Forward declares
	class Core;

	/**
	 * Supported blur kernels
	 */
	enum class EBlurSamples : uint
	{
		X5 = 0,				///< 5x5 kernel, linear sampling
		X9 = 1				///< 9x9 kernel, linear sampling
	};

	/**
	 * Uniform names
	 */
	namespace uniform
	{
		namespace sampler
		{	
			inline constexpr const char* colorTexture	= "colorTexture";		///< Name of the color texture sampler
		}

		inline constexpr const char* uboStruct = "UBO";							///< UBO that contains all the uniforms
		inline constexpr const char* textureSize = "textureSize";				///< Size of the texture
		inline constexpr const char* direction = "direction";					///< Direction of the blur e.g. {1.0, 0.0} for horizontal, {0.0, 1.0} for vertical 
	}

	/**
	 * Gaussian blur shader that samples a Texture2D in a specified direction. For a full gaussian blur,
	 * call this shader twice, setting up a material that samples horizontally (direction = {1.0, 0.0}) and
	 * vertically (direction = {0.0, 1.0}).
	 *
	 * The gaussianblur shader exposes the following shader variables:
	 * 
	 * ~~~~~{.vert}
	 *		uniform UBO
	 *		{
	 *			vec2 textureSize;	// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	 *			vec2 direction;		// The sampling direction
	 *		} ubo;
	 * ~~~~
	 *
	 * ~~~~{.frag}
	 *		uniform sampler2D colorTexture;	// The input color texture to sample from
	 * ~~~~
	 */
	template <EBlurSamples KERNEL>
	class NAPAPI BlurShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:

	    // Constructor
		BlurShader(Core& core);

		/**
		 * Cross compiles the font GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		RenderService* mRenderService = nullptr;
	};

	using Blur5x5Shader = BlurShader<EBlurSamples::X5>;
	using Blur9x9Shader = BlurShader<EBlurSamples::X9>;
}
