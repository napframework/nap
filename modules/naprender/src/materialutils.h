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

		/**
		 * Locates the texture unit that is associated with a specific uniform in a material
		 * You use this index when updating a single texture uniform on the GPU.
		 * This unit is required when calling tex_uniform.push().
		 * @param uniform the texture uniform to find the texture unit number for.
		 * @return the texture unit associated with a specific texture uniform in a material, -1 if not found
		 */
		int NAPAPI getTextureUnit(nap::MaterialInstance& materialInstance, nap::UniformTexture& uniform);
	}
}