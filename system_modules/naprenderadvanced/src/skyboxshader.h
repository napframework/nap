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

	// Skybox shader sampler names
	namespace uniform
	{
		namespace skybox
		{
			inline constexpr const char* cubeTexture = "cubeTexture";			///< Name of the cube texture sampler
			inline constexpr const char* color = "color";						///< color value (0-1)
			inline constexpr const char* alpha = "alpha";						///< alpha value (0-1)
			inline constexpr const char* uboStruct = "UBO";						///< UBO that contains all the uniforms
		}
	}

	/**
	 * SkyBox Shader
	 *
	 * Renders a skybox from a cube texture. Negates the translational component of the view matrix to fake unlimited depth.
	 * Should render a `nap::BoxMesh` as the first object to fill the background. The shader variables are set
	 * automatically using the helper component `nap::RenderSkyBoxComponent`.
	 *
	 * The skybox shader exposes the following shader variables for users:
	 *
	 * ~~~~~{.frag}
	 *	samplerCube	cubeTexture;
	 * ~~~~~
	 */
	class NAPAPI SkyBoxShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		SkyBoxShader(Core& core);

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
