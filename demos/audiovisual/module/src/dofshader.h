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
	class RenderAdvancedService;

	/**
	 * Uniform names
	 */
	namespace uniform
	{
		namespace dof
		{
			namespace sampler
			{	
				inline constexpr const char* colorTexture	= "colorTexture";		///< Name of the color texture sampler
				inline constexpr const char* depthTexture	= "depthTexture";		///< Name of the depth texture sampler
			}

			inline constexpr const char* uboStruct = "UBO";							///< UBO that contains all the uniforms
			inline constexpr const char* textureSize = "textureSize";				///< Size of the texture
			inline constexpr const char* nearFar = "nearFar";
			inline constexpr const char* aperture = "aperture";
			inline constexpr const char* focalLength = "focalLength";
			inline constexpr const char* focusDistance = "focusDistance";
			inline constexpr const char* focusPower = "focusPower";
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
	class NAPAPI DOFShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:

	    // Constructor
		DOFShader(Core& core);

		/**
		 * Cross compiles the font GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		RenderAdvancedService* mRenderAdvancedService = nullptr;
	};
}
