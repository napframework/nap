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
	 * Texture shader. Renders an object using a texture. Set color and alpha to 1.0 to render the texture in its original color.
	 *
	 * The texture shader exposes the following shader variables:
	 *
	 * ~~~~~{.vert}
	 *		uniform UBO
	 *		{
	 *			uniform vec3 color;
	 *			uniform float alpha;
	 *		} ubo;
	 * ~~~~
	 *
	 * ~~~~{.frag}
	 *		uniform sampler2D colorTexture;
	 * ~~~~
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
	
		uint mQuadSampleCount = 8;
		uint mCubeSampleCount = 4;
        bool mEnableEnvironmentMap = true;

	private:
		RenderAdvancedService* mRenderAdvancedService = nullptr;
	};
}
