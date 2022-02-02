#pragma once

// External Includes
#include <shader.h>

namespace nap
{
	// Forward declares
	class Core;
	class RenderService;

	// Video shader sampler names 
	namespace uniform
	{
		namespace constant
		{	
			inline constexpr const char* color			= "color";		///< color value (0-1)
			inline constexpr const char* alpha			= "alpha";		///< alpha value (0-1)
			inline constexpr const char* uboStruct		= "UBO";		///< UBO that contains all the uniforms
		}
	}

	/**
	 * Constant shader. Renders an object using a color and alpha value.
	 * 
	 * The constant shader exposes the following shader variables:
	 *
	 * ~~~~{.frag}
	 *		uniform UBO
	 *		{
	 *			vec3 color;
	 *			float alpha;
	 *		};
	 * ~~~~
	 */
	class NAPAPI ConstantShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		ConstantShader(Core& core);

		/**
		 * Cross compiles the constant GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		RenderService* mRenderService = nullptr;
	};
}
