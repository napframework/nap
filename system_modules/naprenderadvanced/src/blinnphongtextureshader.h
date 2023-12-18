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
	 * Uniform names
	 */
	namespace uniform
	{
		namespace blinnphongtexture
		{
			namespace sampler
			{
				inline constexpr const char* colorTexture = "colorTexture";			///< Name of the color texture sampler
				inline constexpr const char* environmentMap = "environmentMap";		///< Name of the environment map sampler
			}

			inline constexpr const char* ambient = "ambient";						///< Ambient color material property
			inline constexpr const char* diffuse = "diffuse";						///< Diffuse color material property
			inline constexpr const char* specular = "specular";						///< Specular color material property
			inline constexpr const char* fresnel = "fresnel";						///< Fresnel [scale, power]
			inline constexpr const char* shininess = "shininess";					///< Shininess [0, x]
			inline constexpr const char* alpha = "alpha";							///< Alpha [0, 1]
			inline constexpr const char* reflection = "reflection";					///< Reflection [0, 1]
			inline constexpr const char* environment = "environment";				///< Whether to sample an environment map [0, 1]

			inline constexpr const char* uboStruct = "UBO";							///< UBO that contains all the uniforms
		}
	}

	/**
	 * Blinn Phong Texture Shader
	 *
	 * This is NAP's default blinn-phong shader program and is compatible with the `naprenderadvanced` light system.
	 * It also supports shadows and environment maps. When compiling NAP for Raspberry Pi, shadows are disabled when
	 * the Vulkan version is lower than 1.1.
	 *
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
	 *		uniform sampler2D colorTexture;
	 *		uniform samplerCube environmentMap;
	 * ~~~~
	 *
	 * The remaining uniforms are set automatically by the `nap::RenderAdvancedService`.
	 */
	class NAPAPI BlinnPhongTextureShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		BlinnPhongTextureShader(Core& core);

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
