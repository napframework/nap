// Local Includes
#include "material.h"
#include "shaderutils.h"
#include "nshader.h"

// External includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::Material::VertexAttributeBinding)
	RTTI_PROPERTY_REQUIRED("MeshAttributeID",	&nap::Material::VertexAttributeBinding::mMeshAttributeID)
	RTTI_PROPERTY_REQUIRED("ShaderAttributeID", &nap::Material::VertexAttributeBinding::mShaderAttributeID)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Material)
	RTTI_PROPERTY_REQUIRED("Shader",			&nap::Material::mShader)
	RTTI_PROPERTY("VertexAttributeBindings",	&nap::Material::mVertexAttributeBindings)
RTTI_END_CLASS

namespace nap
{
	Material::Material()
	{
	}

	bool Material::init(ErrorState& errorState)
	{
		if (!errorState.check(mShader != nullptr, "Shader not set in material"))
			return false;

		// TODO: store vertex attribute / uniforms for rollback

		uniformAttribute.clear();

		// Add uniforms
		for (const auto& v : mShader->getShader().getUniforms())
		{
			// Make sure we have a valid type for the attribute
			RTTI::TypeInfo attr_value_type = getAttributeType(v.second->mGLSLType);
			if (!errorState.check(attr_value_type != RTTI::TypeInfo::empty(), "unable to map GLSL uniform, unsupported type"))
				return false;

			// Get attribute create function
			const GLSLAttributeCreateFunction* attr_create_function = getAttributeCreateFunction(v.second->mGLSLType);
			if (!errorState.check(attr_create_function != nullptr, "unable to create attribute for GLSL uniform of type: %d, no matching create function found", v.second->mGLSLType))
				return false;

			// Create attribute
			AttributeBase& a = (*attr_create_function)(*(v.second), uniformAttribute);

			// Set attribute name
			a.setName(v.first);
			assert(a.getName() == v.first);
		}

		return true;
	}

	void Material::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			// TODO: implement rollback of uniforms/attrs
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			// TODO: implement rollback of uniforms/attrs
		}
	}

	std::vector<Material::VertexAttributeBinding>& Material::getDefaultVertexAttributeBindings()
	{
		static std::vector<Material::VertexAttributeBinding> bindings;
		if (bindings.empty())
		{
			bindings.push_back({ opengl::Mesh::VertexAttributeIDs::PositionVertexAttr, opengl::Shader::VertexAttributeIDs::PositionVertexAttr });
			bindings.push_back({ opengl::Mesh::VertexAttributeIDs::NormalVertexAttr, opengl::Shader::VertexAttributeIDs::NormalVertexAttr });

			const int numChannels = 4;
			for (int channel = 0; channel != numChannels; ++channel)
			{
				bindings.push_back({ opengl::Mesh::VertexAttributeIDs::GetColorVertexAttr(channel), opengl::Shader::VertexAttributeIDs::GetColorVertexAttr(channel) });
				bindings.push_back({ opengl::Mesh::VertexAttributeIDs::GetUVVertexAttr(channel), opengl::Shader::VertexAttributeIDs::GetUVVertexAttr(channel) });
			}
		}
		return bindings;
	}

	// Unbind shader associated with resource
	void Material::bind()
	{	
		mShader->getShader().bind();
	}


	// Unbind shader associated with resource
	void Material::unbind()
	{
		mShader->getShader().unbind();
	}


	// Upload all uniform variables to GPU
	// TODO, This can be optimized by attaching the uniform directly to the attribute
	void Material::pushUniforms()
	{
		// Set all uniforms
		int tex_unit(0);
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
			const GLSLSetterFunction* func = getGLSLSetFunction(uniform->mGLSLType);
			if (func == nullptr)
			{
				continue;
			}

			// Set value
			(*func)(*uniform, *attr, tex_unit);
		}
		resetActiveTexture();
	}


	// Set link to resource as uniform binding
	void Material::setUniformTexture(const std::string& name, TextureResource& resource)
	{
		AttributeBase* texture_link_attr = uniformAttribute.getAttribute(name);
		if (texture_link_attr == nullptr)
		{
			nap::Logger::warn(*this, "uniform variable: %s does not exist", name.c_str());
			return;
		}

		if (!texture_link_attr->get_type().is_derived_from(RTTI_OF(ResourceLinkAttribute)))
		{
			nap::Logger::warn(*this, "uniform variable: %s is not a resource link");
			return;
		}

		// Set resource
		ResourceLinkAttribute* resource_link = static_cast<ResourceLinkAttribute*>(texture_link_attr);
		resource_link->setResource(resource);
	}

	const Material::VertexAttributeBinding* Material::findVertexAttributeBinding(const opengl::Mesh::VertexAttributeID& shaderAttributeID) const
	{
		for (const VertexAttributeBinding& binding : mVertexAttributeBindings)
			if (binding.mShaderAttributeID == shaderAttributeID)
				return &binding;

		return nullptr;
	}
}