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

	/**
	 * Supported blur kernels
	 */
	enum class EBlurSamples : uint
	{
		X5,				///< 5x5 kernel, linear sampling
		X9				///< 9x9 kernel, linear sampling
	};

	/**
	 * Uniform names
	 */
	namespace uniform
	{
		namespace sampler
		{	
			inline constexpr const char* colorTexture	= "colorTexture";		///< Name of the color texture sampler
		}

		inline constexpr const char* uboStruct = "UBO";							///< UBO that contains all the uniforms
		inline constexpr const char* textureSize = "textureSize";				///< Size of the texture
		inline constexpr const char* direction = "direction";					///< Direction of the blur e.g. {1.0, 0.0} for horizontal, {0.0, 1.0} for vertical 
	}

	/**
	 * Shader that blurs a texture in one direction
	 */
	template <EBlurSamples KERNEL>
	class NAPAPI BlurShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:

		BlurShader(Core& core) : Shader(core),
			mRenderService(core.getService<RenderService>()) { }

		/**
		 * Cross compiles the font GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override
		{
			std::string vertex_shader_path = (KERNEL == EBlurSamples::X5) ?
				mRenderService->getModule().findAsset(blurVert) :
				mRenderService->getModule().findAsset(blur9Vert);

			if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find blur vertex shader %s", mRenderService->getModule().getName().c_str(), vertex_shader_path.c_str()))
				return false;

			std::string fragment_shader_path = (KERNEL == EBlurSamples::X5) ? 
				mRenderService->getModule().findAsset(blurFrag) :
				mRenderService->getModule().findAsset(blur9Frag);

			if (!errorState.check(!fragment_shader_path.empty(), "%s: Unable to find blur fragment shader %s", mRenderService->getModule().getName().c_str(), fragment_shader_path.c_str()))
				return false;

			// Read vert shader file
			std::string vert_source;
			if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read blur vertex shader file"))
				return false;

			// Read frag shader file
			std::string frag_source;
			if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read blur fragment shader file"))
				return false;

			// Compile shader
			std::string shader_name = utility::getFileNameWithoutExtension(blurVert);
			return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
		}

	private:
		RenderService* mRenderService = nullptr;

		//////////////////////////////////////////////////////////////////////////
		// Shader path literals
		//////////////////////////////////////////////////////////////////////////

		static inline char* blurVert = "shaders/gaussianblur.vert";
		static inline char* blurFrag = "shaders/gaussianblur.frag";

		static inline char* blur9Vert = "shaders/gaussianblur9.vert";
		static inline char* blur9Frag = "shaders/gaussianblur9.frag";
	};

	using Blur5x5Shader = BlurShader<EBlurSamples::X5>;
	using Blur9x9Shader = BlurShader<EBlurSamples::X9>;
}
