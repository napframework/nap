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
		namespace texture
		{
			namespace sampler
			{
				inline constexpr const char* colorTexture = "colorTexture";		///< Name of the color texture sampler
			}

			inline constexpr const char* color			= "color";		///< color value (0-1)
			inline constexpr const char* alpha			= "alpha";		///< alpha value (0-1)
			inline constexpr const char* uboStruct		= "UBO";		///< UBO that contains all the uniforms
		}
	}

	/**
	 * Texture shader. Renders an object using a texture. Set color and alpha to 1.0 to render the texture in its original color.
	 *
	 * The texture shader exposes the following shader variables:
	 *
	 * ~~~~{.vert}
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
	class NAPAPI TextureShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		TextureShader(Core& core);

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
