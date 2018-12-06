// Local Includes
#include "materialutils.h"

// External Includes
#include <GL/glew.h>

namespace nap
{
	namespace utility
	{
		void NAPAPI pushUniforms(nap::MaterialInstance& materialInstance)
		{
			Material* material = materialInstance.getMaterial();

			// Keep track of which uniforms were set (i.e. overridden) by the material instance
			std::unordered_set<std::string> instance_bindings;
			int texture_unit = 0;

			// Push all uniforms that are set (i.e. overridden) in the instance
			const UniformTextureBindings& instance_texture_bindings = materialInstance.getTextureBindings();
			for (auto& kvp : instance_texture_bindings)
			{
				nap::Uniform* uniform_tex = kvp.second.mUniform.get();
				assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
				static_cast<nap::UniformTexture*>(uniform_tex)->push(*kvp.second.mDeclaration, texture_unit++);
				instance_bindings.insert(kvp.first);
			}

			const UniformValueBindings& instance_value_bindings = materialInstance.getValueBindings();
			for (auto& kvp : instance_value_bindings)
			{
				nap::Uniform* uniform_tex = kvp.second.mUniform.get();
				assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformValue)));
				static_cast<nap::UniformValue*>(uniform_tex)->push(*kvp.second.mDeclaration);
				instance_bindings.insert(kvp.first);
			}

			// Push all uniforms in the material that weren't overridden by the instance
			// Note that the material contains mappings for all the possible uniforms in the shader
			for (auto& kvp : material->getTextureBindings())
			{
				if (instance_bindings.find(kvp.first) == instance_bindings.end())
				{
					nap::Uniform* uniform_val = kvp.second.mUniform.get();
					assert(uniform_val->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
					static_cast<nap::UniformTexture*>(uniform_val)->push(*kvp.second.mDeclaration, texture_unit++);
				}

			}
			for (auto& kvp : material->getValueBindings())
			{
				if (instance_bindings.find(kvp.first) == instance_bindings.end())
				{
					nap::Uniform* uniform_val = kvp.second.mUniform.get();
					assert(uniform_val->get_type().is_derived_from(RTTI_OF(nap::UniformValue)));
					static_cast<nap::UniformValue*>(uniform_val)->push(*kvp.second.mDeclaration);
				}
			}

			glActiveTexture(GL_TEXTURE0);
		}


		void NAPAPI setBlendMode(nap::MaterialInstance& materialInstance)
		{
			EDepthMode depth_mode = materialInstance.getDepthMode();
			glDepthFunc(GL_LEQUAL);
			glBlendEquation(GL_FUNC_ADD);
			switch (materialInstance.getBlendMode())
			{
			case EBlendMode::Opaque:
			{
				glDisable(GL_BLEND);
				if (depth_mode == EDepthMode::InheritFromBlendMode)
				{
					glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_TRUE);
				}
				break;
			}
			case EBlendMode::AlphaBlend:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				if (depth_mode == EDepthMode::InheritFromBlendMode)
				{
					glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_FALSE);
				}
				break;
			}
			case EBlendMode::Additive:
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				if (depth_mode == EDepthMode::InheritFromBlendMode)
				{
					glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_FALSE);
				}
				break;
			}
			}

			if (depth_mode != EDepthMode::InheritFromBlendMode)
			{
				switch (depth_mode)
				{
				case EDepthMode::ReadWrite:
				{
					glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_TRUE);
					break;
				}
				case EDepthMode::ReadOnly:
				{
					glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_FALSE);
					break;
				}
				case EDepthMode::WriteOnly:
				{
					glDisable(GL_DEPTH_TEST);
					glDepthMask(GL_TRUE);
					break;
				}
				case EDepthMode::NoReadWrite:
				{
					glDisable(GL_DEPTH_TEST);
					glDepthMask(GL_FALSE);
					break;
				}
				default:
					assert(false);
				}
			}
		}

	}
}