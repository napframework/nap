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
		namespace font
		{	
			constexpr const char* glyphSampler	= "glyph";		///< Name of the 2D sampler that points to the glyph
			constexpr const char* uboStruct		= "UBO";		///< UBO that contains all the uniforms
			constexpr const char* textColor		= "textColor";	///< Text color vec3 
		}
	}

	/**
	 * Shader that renders glyphs. Used by the nap::RenderableTextComponent
	 */
	class NAPAPI FontShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		FontShader(Core& core);

		/**
		 * Cross compiles the font GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}