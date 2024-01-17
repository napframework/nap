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
			inline constexpr const char* direction = "direction";					///< Blur direction
			inline constexpr const char* nearFar = "nearFar";						///< Camera near/far planes
			inline constexpr const char* aperture = "aperture";						///< Influences the area of focus
			inline constexpr const char* focalLength = "focalLength";				///< Distance to focus point
			inline constexpr const char* focusDistance = "focusDistance";			///< Focal length
			inline constexpr const char* focusPower = "focusPower";					///< Additional power over circle of confusion
		}
	}

	/**
	 * Depth-of-field shader that performs a gaussian blur based on the depth value of a fragment.
	 * Call this shader twice to sample horizontally (direction = {1.0, 0.0}) and vertically (direction = {0.0, 1.0}).
	 *
	 * The dof shader exposes the following shader variables:
	 * 
	 * ~~~~~{.vert}
	 *		uniform UBO
	 *		{
	 *			vec2 textureSize;				// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	 *			vec2 direction;					// The sampling direction
	 * 			vec2 nearFar;					// camera near/far planes
	 *			float aperture;					// Influences the area of focus
	 *			float focusDistance;			// Distance to focus point
	 *			float focalLength;				// Focal length
	 *			float focusPower;				// Additional power over circle of confusion
	 *		} ubo;
	 * ~~~~
	 *
	 * ~~~~{.frag}
	 *		uniform sampler2D colorTexture;		// The input color texture to sample from
	 * 		uniform sampler2D depthTexture;		// The input depth texture to sample from
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
