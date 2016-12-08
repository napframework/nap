// Include materialcomponent
#include "material.h"

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
		nap::Object* obj = shaderResource.getTarget();
		if (obj == nullptr)
			return nullptr;
		return static_cast<ShaderResource*>(obj);
	}

}

RTTI_DEFINE(nap::Material)