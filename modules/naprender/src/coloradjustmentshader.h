/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "shader.h"

namespace nap
{
	// Forward declares
	class Core;
	class Material;
    class RenderService;

	// Color adjustment uniform shader names
	namespace uniform
	{
		namespace coloradjustment
		{
			namespace sampler
			{
				inline constexpr const char* colorTexture = "colorTexture";			///< Name of the color texture sampler
			}

			inline constexpr const char* uboStruct = "UBO";							///< UBO that contains all the uniforms
			inline constexpr const char* brightness = "brightness";					///< Controls brightness [-x, x]
			inline constexpr const char* contrast = "contrast";						///< Controls contrast [-1.0, 1.0]
			inline constexpr const char* saturation = "saturation";					///< Controls saturation [0.0, x]
		}
	}

	/**
	 * Shader that applies a contrast, brightness and saturation filter to a texture.
	 *
	 * The coloradjustment shader exposes the following shader variables:
	 *
	 * ~~~~{.frag}
	 *		uniform UBO
	 *		{
	 *			float contrast;		// Contrast adjustment [-1.0, 1.0]
	 *			float brightness;	// Brightness adjustment [-x, x]
	 *			float saturation;	// Saturation adjustment [0.0, x]
	 *		};
	 *
	 *		uniform sampler2D colorTexture;
	 * ~~~~
	 */
	class NAPAPI ColorAdjustmentShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		ColorAdjustmentShader(Core& core);

		/**
		 * Cross compiles the font GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		RenderService* mRenderService = nullptr;
	};
}
