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

	// Cube map shader sampler names 
	namespace uniform
	{
		namespace cubemap
		{
			namespace sampler
			{
				inline constexpr const char* equiTexture = "equiTexture";		///< Name of the texture sampler
			}

			inline constexpr const char* uboStruct = "UBO";						///< Name of the uniform struct
			inline constexpr const char* face = "face";							///< Name of the face index uniform
		}
	}

	/**
	 * Cube Map shader.
	 *
	 * The cube map shader exposes the following shader variables:
	 *
	 * ~~~~~{.frag}
	 * uniform UBO
	 * {
	 *		uint face;
	 * } ubo;
	 * 
	 * uniform sampler2D equiTexture;
	 * ~~~~~
	 *
	 */
	class NAPAPI CubeMapShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		CubeMapShader(Core& core);

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
