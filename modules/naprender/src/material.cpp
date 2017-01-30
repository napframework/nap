// Include materialcomponent
#include "material.h"

// External includes
#include <nap/logger.h>

namespace nap
{
	Material::Material()
	{
		// Listen to when the shader changes, if so we want to
		// make sure we listen to when it is compiled
		shaderResourceLink.valueChanged.connect(shaderResourceChanged);

		// Add uniform values
		AttributeObject* uniforms_obj = &this->addChild<AttributeObject>("uniforms");
		uniforms_obj->setFlag(nap::ObjectFlag::Removable, false);
		uniforms.setTarget(*uniforms_obj);
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


	// Resolve uniforms
	void Material::resolveUniforms()
	{
		// Make sure the resource is valid
		if (!hasShader())
		{
			assert(false);
			nap::Logger::warn(*this, "unable to resolve shader uniforms, no shader found");
			return;
		}

		// Make sure it's loaded
		if (!mShader->isLoaded())
		{
			assert(false);
			nap::Logger::warn(*this, "attempting to resolve shader uniforms for incomplete shader: %s", mShader->getName().c_str());
			return;
		}

		// Resolve
		nap::Logger::info(*this, "resolving all shader uniform variables");
		AttributeObject* uniform_attrs = uniforms.getTarget<AttributeObject>();
		for (const auto& v : mShader->getShader().getUniforms())
		{
			uniform_attrs->addAttribute<float>(v.second->mName, 1.0f);
		}
	}


	// Clears all uniforms
	void Material::clearUniforms()
	{
		uniforms.getTarget<AttributeObject>()->clearChildren();
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