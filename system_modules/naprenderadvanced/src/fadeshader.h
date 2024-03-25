/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <shader.h>

namespace nap
{
	// Forward declares
	class Core;
	class RenderAdvancedService;

	// FadeShader sampler names 
	namespace uniform
	{
		namespace fade
		{
			inline constexpr const char* uboStruct = "UBO";						///< Name of the uniform struct
			inline constexpr const char* color = "color";						///< Name of the color uniform
			inline constexpr const char* alpha = "alpha";						///< Name of the alpha uniform
		}
	}

	/**
	 * Fade shader. Fills the screen with a triangle of the given color.
	 *
	 * The fade shader exposes the following shader variables:
	 *
	 * ~~~~~{.vert}
	 *		uniform UBO
	 *		{
	 *			vec3 color;
	 *			float alpha;
	 *		} ubo;
	 * ~~~~
	 */
	class NAPAPI FadeShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		FadeShader(Core& core);

		/**
		/**
		 * Cross compiles the constant GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		RenderAdvancedService* mRenderAdvancedService = nullptr;
	};
}
