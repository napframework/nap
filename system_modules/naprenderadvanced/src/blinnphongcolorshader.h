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

	/**
	 * Blinn Phong Color Shader
	 *
	 * This shader is compatible with the `naprenderadvanced` light system.
	 * The blinnphongcolor shader exposes the following shader variables for users:
	 *
	 * ~~~~~{.vert}{.frag}
	 * uniform UBO
	 * {
	 *		vec4	ambient;						//< Ambient
	 * 		vec3	diffuse;						//< Diffuse
	 * 		vec3	specular;						//< Specular
	 * 		vec2	fresnel;						//< Fresnel [scale, power]
	 * 		float	shininess;						//< Shininess
	 * 		float	alpha;							//< Alpha
	 *		float	reflection;						//< Reflection
	 *		uint	environment;					//< Whether to sample an environment map
	 * } ubo;
	 * ~~~~
	 *
	 * ~~~~{.frag}
	 *		uniform samplerCube environmentMap;
	 * ~~~~
	 *
	 * The remaining uniforms are set automatically by the `nap::RenderAdvancedService`.
	 */
	class NAPAPI BlinnPhongColorShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		BlinnPhongColorShader(Core& core);

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
