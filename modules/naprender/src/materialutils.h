#pragma once

#include <utility/dllexport.h>
#include <material.h>

namespace nap
{
	namespace utility
	{
		/**
		 * Uploads all uniforms variables of a material to the GPU.
		 * This applies to the uniforms in the instance that are overridden as for the uniforms in the underlying material.
		 * @param material the material instance to push all the variables for.
		 */
		void NAPAPI pushUniforms(nap::MaterialInstance& materialInstance);

		/**
		 * Sets the OpenGL blend mode based on the blend mode settings in the material instance.
		 * @param materialInstance the material to set the blend mode in OpenGL for
		 */
		void NAPAPI setBlendMode(nap::MaterialInstance& materialInstance);
	}
}