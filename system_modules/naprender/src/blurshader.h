/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <shader.h>
#include <renderservice.h>
#include <nap/core.h>

namespace nap
{
	// Forward declares
	class Core;

	/**
	 * Supported blur kernels
	 */
	enum class EBlurSamples : uint
	{
		X5	= 0,	///< 5x5 kernel, linear sampling
		X9	= 1,	///< 9x9 kernel, linear sampling
		X13 = 2		///< 13x13 kernel, linear sampling
	};

	/**
	 * Uniform names
	 */
	namespace uniform
	{
		namespace blur
		{
			namespace sampler
			{	
				inline constexpr const char* colorTexture	= "colorTexture";		///< Name of the color texture sampler
			}

			inline constexpr const char* uboStruct = "UBO";							///< UBO that contains all the uniforms
			inline constexpr const char* textureSize = "textureSize";				///< Size of the texture
			inline constexpr const char* direction = "direction";					///< Direction of the blur e.g. {1.0, 0.0} for horizontal, {0.0, 1.0} for vertical 
		}
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
	class BlurShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
	    // Constructor
		BlurShader(Core& core) : Shader(core), mRenderService(core.getService<RenderService>()) { }

		/**
		 * Cross compiles the font GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		RenderService* mRenderService = nullptr;
	};

	// Blur Shader Definitions
	using Blur5x5Shader = BlurShader<EBlurSamples::X5>;
	using Blur9x9Shader = BlurShader<EBlurSamples::X9>;
	using Blur13x13Shader = BlurShader<EBlurSamples::X13>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template <EBlurSamples KERNEL>
	bool nap::BlurShader<KERNEL>::init(utility::ErrorState& errorState)
	{
		if (!Shader::init(errorState))
			return false;

		std::string shader_name;
		switch (KERNEL)
		{
		case nap::EBlurSamples::X5:
			shader_name = "gaussianblur5";
			break;
		case nap::EBlurSamples::X9:
			shader_name = "gaussianblur9";
			break;
		case nap::EBlurSamples::X13:
			shader_name = "gaussianblur13";
			break;
		default:
			errorState.fail("Unknown blur sample type.");
			return false;
		}
		return loadDefault(shader_name, errorState);
	}
}
