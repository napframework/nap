// Include materialcomponent
#include "material.h"

// External includes
#include <nap/logger.h>

namespace nap
{
	// Unbind shader associated with resource
	bool Material::bind()
	{	
		ShaderResource* resource = getResource();
		if(resource == nullptr)
		{
			nap::Logger::warn("unable to bind shader instance, no resource linked");
			return false;
		}
		return resource->getShader().bind();
	}


	// Unbind shader associated with resource
	bool Material::unbind()
	{
		ShaderResource* resource = getResource();
		if (resource == nullptr)
		{
			nap::Logger::warn("unable to unbind shader instance, no resource linked");
			return false;
		}
		return resource->getShader().unbind();
	}


	// Return link as shader resource, nullptr if not linked
	ShaderResource* Material::getResource()
	{
		return shaderResource.getResource<ShaderResource>();
	}

}

RTTI_DEFINE(nap::Material)