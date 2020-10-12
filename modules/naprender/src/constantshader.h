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
		namespace constant
		{	
			constexpr const char* color			= "color";		///< Name of the 2D sampler that points to the glyph
			constexpr const char* alpha			= "alpha";		///< Text color vec3 
			constexpr const char* uboStruct		= "UBO";		///< UBO that contains all the uniforms
		}
	}

	/**
	 * Constant shader. Renders an object using a color and alpha value.
	 */
	class NAPAPI ConstantShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		ConstantShader(Core& core);

		/**
		 * Cross compiles the constant GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}