// Local Includes
#include "material.h"
#include "shaderutils.h"

// External includes
#include <nap/logger.h>

namespace nap
{
	Material::Material()
	{
		// Listen to when the shader changes, if so we want to
		// make sure we listen to when it is compiled
		shaderResourceLink.valueChanged.connect(shaderResourceChanged);
	}


	// Unbind shader associated with resource
	bool Material::bind()
	{	
		if(!hasShader())
		{
			nap::Logger::warn("unable to bind shader instance, no resource linked");
			return false;
		}
		return mShader->getShader().bind();
	}


	// Unbind shader associated with resource
	bool Material::unbind()
	{
		if (!hasShader())
		{
			nap::Logger::warn("unable to unbind shader instance, no resource linked");
			return false;
		}
		return mShader->getShader().unbind();
	}


	// Upload all uniform variables to GPU
	// TODO, This can be optimized by attaching the uniform directly to the attribute
	void Material::pushUniforms()
	{
		// Ensure shader exists
		if (!hasShader())
		{
			nap::Logger::warn(*this, "unable to set uniform attribute, no resource linked");
			return;
		}

		// Set all uniforms
		for (auto i = 0; i < uniformAttribute.size(); i++)
		{
			// Get attribute to set
			AttributeBase* attr = uniformAttribute.getAttribute(i);
			assert(attr != nullptr);
			
			// Get matching uniform
			const opengl::UniformVariable* uniform = mShader->getShader().getUniform(attr->getName());
			if (uniform == nullptr)
			{
				nap::Logger::warn(*this, "unable set uniform GLSL attribute: %s, no attribute found", attr->getName().c_str());
				continue;
			}
			
			// Get GLSL set function
			const GLSLSetterFunction* func = getGLSLSetFunction(attr->getValueType());
			if (func == nullptr)
			{
				continue;
			}

			// Set value
			(*func)(*uniform, *attr);
		}
	}


	// Resolve uniforms
	void Material::resolveUniforms()
	{
		// Clear, TODO: ACTUALLY RESOLVE
		uniformAttribute.clear();

		// Resolve can only occur when a shader is present
		assert(mShader != nullptr);

		// Make sure it's loaded
		if (!mShader->isLoaded())
		{
			assert(false);
			nap::Logger::warn(*this, "attempting to resolve shader uniforms for incomplete shader: %s", mShader->getName().c_str());
			return;
		}

		// Add
		for (const auto& v : mShader->getShader().getUniforms())
		{
			// Make sure we have a valid type for the attribute
			RTTI::TypeInfo attr_value_type = getAttributeType(v.second->mGLSLType);
			if (attr_value_type == RTTI::TypeInfo::empty())
			{
				nap::Logger::warn(*this, "unable to map GLSL uniform: %s, unsupported type");
				continue;
			}

			// Get attribute create function
			const GLSLAttributeCreateFunction* attr_create_function = getAttributeCreateFunction(v.second->mGLSLType);
			if (attr_create_function == nullptr)
			{
				nap::Logger::warn(*this, "unable to create attribute for GLSL uniform of type: %d, no matching create function found", v.second->mGLSLType);
				continue;
			}
			
			// Create attribute
			AttributeBase& a = (*attr_create_function)(*(v.second), uniformAttribute);
		}
	}


	// Clears all uniforms
	void Material::clearUniforms()
	{
		uniformAttribute.clearChildren();
	}


	// Occurs when the link is set or cleared
	void Material::onResourceLinkChanged(AttributeBase& value)
	{		
		resolveShaderLink();
	}


	// Update uniform variables
	void Material::onShaderLoaded(bool success)
	{
		// If the shader failed to load, clear bindings
		if (!success)
		{
			nap::Logger::warn(*this, "failed to load shader: %s, clearing uniform bindings", mShader->getResourcePath().c_str());
			clearUniforms();
			return;
		}

		// Otherwise we resolve bindings
		resolveUniforms();
	}


	// Tries to resolve the link to the shader
	void Material::resolveShaderLink()
	{
		// If we currently link to a resource we want to stop 
		// Listening to that one, it also means that the resource
		// is different and we can safely destroy all uniform bindings
		// so that if we resolve we resolve against a fresh set
		if (hasShader())
		{
			clearUniforms();
			mShader->loaded.disconnect(shaderLoaded);
		}

		// Store the new resource
		mShader = shaderResourceLink.getResource<ShaderResource>();

		// If the resource points to nothing we simply clear
		// all available uniform shader attributes
		if (mShader == nullptr)
		{
			if (shaderResourceLink.isLinked())
			{
				nap::Logger::fatal(*this, "unable to resolve shader link: %s", shaderResourceLink.getPath().c_str());
			}
			clearUniforms();
			return;
		}

		// If there is a new resource, connect to it's loaded signal
		// When the shader is loaded we want to update our uniform bindings
		mShader->loaded.connect(shaderLoaded);

		// If the resource is already loaded and loaded correctly
		// we can immediately update all the uniforms
		if (mShader->isLoaded())
		{
			resolveUniforms();
		}
	}
}

RTTI_DEFINE(nap::Material)