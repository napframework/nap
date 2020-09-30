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
	class Material;

	// Video shader sampler names 
	namespace uniform
	{
		namespace video
		{
			constexpr const char* YSampler = "yTexture";	///< video shader Y sampler name
			constexpr const char* USampler = "uTexture";	///< video shader U sampler name
			constexpr const char* VSampler = "vTexture";	///< video shader V sampler name
		}
	}

	/**
	 * Shader that converts YUV video textures, output by the nap::VideoPlayer, into an RGB image.
	 * Used by the nap::RenderVideoComponent.
	 */
	class NAPAPI VideoShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		VideoShader(Core& core);

		/**
		 * Cross compiles the video GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}
