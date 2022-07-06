#pragma once

// External Includes
#include <shader.h>

namespace nap
{
	// Forward declares
	class Core;
	class Material;


	//////////////////////////////////////////////////////////////////////////
	// LightFieldShader Variables
	//////////////////////////////////////////////////////////////////////////

	namespace uniform
	{
		namespace lightfield
		{
			constexpr const char* UBO = "UBO";

			// Calibration
			constexpr const char* pitch = "pitch";
			constexpr const char* tilt = "tilt";
			constexpr const char* center = "center";
			constexpr const char* invView = "invView";
			constexpr const char* subp = "subp";
			constexpr const char* displayAspect = "displayAspect";
			constexpr const char* ri = "ri";
			constexpr const char* bi = "bi";

			// Quilt settings
			constexpr const char* tile = "tile";
			constexpr const char* viewPortion = "viewPortion";
			constexpr const char* quiltAspect = "quiltAspect";
			constexpr const char* overscan = "overscan";
			constexpr const char* quiltInvert = "quiltInvert";

			constexpr const char* debug = "debug";
		}
	}

	namespace sampler
	{
		namespace lightfield
		{
			constexpr const char* screenTex = "screenTex";
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// LightFieldShader
	//////////////////////////////////////////////////////////////////////////

	/**
	 * LightFieldShader
	 *
	 * A shader resource for the Holoplay Core SDK lightfield shader. Converts a quilt texture to a light field for the
	 * Looking Glass. Used by nap::RenderLightFieldComponent.
	 *
	 * Some modifications have been made to the original shader included in the SDK to make the program compatible with
	 * Vulkan and NAP. These include moving the configuration uniform variables in UBO declarations, and simplifying the
	 * vertex shader to use no buffers. It is important to be aware of this if the lightfield shader is updated in the
	 * future.
	 *
	 * This shader is a post-processing operation and requires no vertex attribute buffers. Bind the material this shader
	 * is created with to a nap::DummyMesh included in the lookingglass module.
	 *
	 * A quick explainer on light field capture from lookingglassfactory:
	 * https://docs.lookingglassfactory.com/keyconcepts/capturing-a-lightfield/linear-light-field-capture
	 */
	class NAPAPI LightFieldShader : public Shader
	{
		RTTI_ENABLE(Shader)
	public:
		LightFieldShader(Core& core);

		/**
		 * Cross compiles the constant GLSL shader code to SPIR-V, creates the shader module and parses all the uniforms and samplers.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}
