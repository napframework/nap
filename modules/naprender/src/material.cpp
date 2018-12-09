// Local Includes
#include "material.h"
#include "nshader.h"
#include "mesh.h"

// External includes
#include <nap/logger.h>
#include <GL/glew.h>
#include "rtti/rttiutilities.h"

RTTI_BEGIN_ENUM(nap::EBlendMode)
	RTTI_ENUM_VALUE(nap::EBlendMode::NotSet,				"NotSet"),
	RTTI_ENUM_VALUE(nap::EBlendMode::Opaque,				"Opaque"),
	RTTI_ENUM_VALUE(nap::EBlendMode::AlphaBlend,			"AlphaBlend"),
	RTTI_ENUM_VALUE(nap::EBlendMode::Additive,				"Additive")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EDepthMode)
	RTTI_ENUM_VALUE(nap::EDepthMode::NotSet,				"NotSet"),
	RTTI_ENUM_VALUE(nap::EDepthMode::InheritFromBlendMode,	"InheritFromBlendMode"),
	RTTI_ENUM_VALUE(nap::EDepthMode::ReadWrite,				"ReadWrite"),
	RTTI_ENUM_VALUE(nap::EDepthMode::ReadOnly,				"ReadOnly"),
	RTTI_ENUM_VALUE(nap::EDepthMode::WriteOnly,				"WriteOnly"),
	RTTI_ENUM_VALUE(nap::EDepthMode::NoReadWrite,			"NoReadWrite")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::Material::VertexAttributeBinding)
	RTTI_VALUE_CONSTRUCTOR(const std::string&, const std::string&)
	RTTI_PROPERTY("MeshAttributeID",			&nap::Material::VertexAttributeBinding::mMeshAttributeID, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ShaderAttributeID",			&nap::Material::VertexAttributeBinding::mShaderAttributeID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::MaterialInstanceResource)
	RTTI_PROPERTY("Material",					&nap::MaterialInstanceResource::mMaterial,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Uniforms",					&nap::MaterialInstanceResource::mUniforms,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("BlendMode",					&nap::MaterialInstanceResource::mBlendMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",					&nap::MaterialInstanceResource::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MaterialInstance)
	RTTI_FUNCTION("getOrCreateUniform",			(nap::Uniform* (nap::MaterialInstance::*)(const std::string&)) &nap::MaterialInstance::getOrCreateUniform);
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Material)
	RTTI_PROPERTY("Uniforms",					&nap::Material::mUniforms,					nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Shader",						&nap::Material::mShader,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VertexAttributeBindings",	&nap::Material::mVertexAttributeBindings,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendMode",					&nap::Material::mBlendMode,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",					&nap::Material::mDepthMode,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	/**
	* Creates Uniform objects based on a declaration.
	*/
	std::unique_ptr<Uniform> createUniformFromDeclaration(const opengl::UniformDeclaration& declaration)
	{
		std::unique_ptr<Uniform> result;

		switch (declaration.mGLSLType)
		{
		case opengl::EGLSLType::Int:
			result = std::make_unique<UniformInt>();
			break;
		case opengl::EGLSLType::Float:
			result = std::make_unique<UniformFloat>();
			break;
		case opengl::EGLSLType::Vec4:
			result = std::make_unique<UniformVec4>();
			break;
		case opengl::EGLSLType::Mat4:
			result = std::make_unique<UniformMat4>();
			break;
		case opengl::EGLSLType::Tex2D:
			result = std::make_unique<UniformTexture2D>();
			break;
		case opengl::EGLSLType::Vec3:
			result = std::make_unique<UniformVec3>();
			break;
		}
		assert(result);
		result->mName = declaration.mName;
		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	// MaterialInstance
	//////////////////////////////////////////////////////////////////////////

	Uniform* MaterialInstance::getOrCreateUniform(const std::string& name)
	{
		Uniform* existing = findUniform(name);
		if (existing != nullptr)
			return existing;

		return &createUniform(name);
	}


	void MaterialInstance::bind()
	{
		getMaterial()->bind();
	}


	void MaterialInstance::unbind()
	{
		getMaterial()->unbind();
	}


	void MaterialInstance::pushUniforms()
	{
		// Keep track of which uniforms were set (i.e. overridden) by the material instance
		std::unordered_set<std::string> instance_bindings;
		int texture_unit = 0;

		// Push all texture uniforms that are set (i.e. overridden) in the instance
		const UniformTextureBindings& instance_texture_bindings = getTextureBindings();
		for (auto& kvp : instance_texture_bindings)
		{
			nap::Uniform* uniform_tex = kvp.second.mUniform.get();
			assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
			static_cast<nap::UniformTexture*>(uniform_tex)->push(*kvp.second.mDeclaration, texture_unit++);
			instance_bindings.insert(kvp.first);
		}

		// Push all value uniforms that are set (i.e. overridden) in the instance
		const UniformValueBindings& instance_value_bindings = getValueBindings();
		for (auto& kvp : instance_value_bindings)
		{
			nap::Uniform* uniform_val = kvp.second.mUniform.get();
			assert(uniform_val->get_type().is_derived_from(RTTI_OF(nap::UniformValue)));
			static_cast<nap::UniformValue*>(uniform_val)->push(*kvp.second.mDeclaration);
			instance_bindings.insert(kvp.first);
		}

		// Push all uniform textures in the material that weren't overridden by the instance
		// Note that the material contains mappings for all the possible uniforms in the shader
		Material* material = getMaterial();
		for (auto& kvp : material->getTextureBindings())
		{
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
			{
				nap::Uniform* uniform_tex = kvp.second.mUniform.get();
				assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
				static_cast<nap::UniformTexture*>(uniform_tex)->push(*kvp.second.mDeclaration, texture_unit++);
			}

		}

		// Push all uniform values in the material that weren't overridden by the instance
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


	void MaterialInstance::pushBlendMode()
	{
		EDepthMode depth_mode = getDepthMode();
		
		// Global
		glDepthFunc(GL_LEQUAL);
		glBlendEquation(GL_FUNC_ADD);

		// Switch based on blend mode
		switch (getBlendMode())
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

		// If the depth mode is not inherited (based on blend mode) set it.
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


	int MaterialInstance::getTextureUnit(nap::UniformTexture& uniform)
	{
		int texture_unit = 0;
		std::unordered_set<std::string> instance_bindings;

		// Iterate over all material instance texture bindings
		// If the texture uniform matches the requested uniform it is considered to be at that location
		// If not the location is incremented until a match is found
		const UniformTextureBindings& instance_texture_bindings = getTextureBindings();
		for (auto& kvp : instance_texture_bindings)
		{
			nap::Uniform* uniform_tex = kvp.second.mUniform.get();
			if (uniform_tex == &uniform)
				return texture_unit;
			texture_unit++;
			instance_bindings.insert(kvp.first);
		}

		// Iterate over all source material bindings
		// If the texture uniform matches the requested uniform it is considered to be at that location
		// If not, there is no valid texture binding associated with the given uniform
		Material* material = getMaterial();
		for (auto& kvp : material->getTextureBindings())
		{
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
			{
				nap::Uniform* uniform_tex = kvp.second.mUniform.get();
				if (uniform_tex == &uniform)
					return texture_unit;
				texture_unit++;
			}
		}

		// No texture binding associated with the given uniform
		return -1;
	}


	Uniform& MaterialInstance::createUniform(const std::string& name)
	{
		const opengl::UniformDeclarations& uniform_declarations = mResource->mMaterial->getShader()->getShader().getUniformDeclarations();

		opengl::UniformDeclarations::const_iterator pos = uniform_declarations.find(name);
		assert(pos != uniform_declarations.end());

		const opengl::UniformDeclaration* declaration = pos->second.get();
		std::unique_ptr<Uniform> uniform = createUniformFromDeclaration(*declaration);
		return AddUniform(std::move(uniform), *declaration);
	}


	bool MaterialInstance::init(MaterialInstanceResource& resource, utility::ErrorState& errorState)
	{
		mResource = &resource;
		const opengl::UniformDeclarations& uniform_declarations = resource.mMaterial->getShader()->getShader().getUniformDeclarations();

		// Create new uniforms for all the uniforms in mUniforms
		for (ResourcePtr<Uniform>& uniform : resource.mUniforms)
		{
			opengl::UniformDeclarations::const_iterator declaration = uniform_declarations.find(uniform->mName);
			if (declaration == uniform_declarations.end())
				continue;

			if (!errorState.check(uniform->getGLSLType() == declaration->second->mGLSLType, "Uniform %s does not match the variable type in the shader %s", uniform->mName.c_str(), resource.mMaterial->getShader()->mID.c_str()))
				return false;

			std::unique_ptr<Uniform> new_uniform = createUniformFromDeclaration(*declaration->second);
			nap::rtti::copyObject(*uniform, *new_uniform.get());

			AddUniform(std::move(new_uniform), *declaration->second);
		}

		return true;
	}


	Material* MaterialInstance::getMaterial() 
	{ 
		return mResource->mMaterial.get(); 
	}


	EBlendMode MaterialInstance::getBlendMode() const
	{
		if (mResource->mBlendMode != EBlendMode::NotSet)
			return mResource->mBlendMode;

		return mResource->mMaterial->getBlendMode();
	}


	void MaterialInstance::setBlendMode(EBlendMode blendMode)
	{
		mResource->mBlendMode = blendMode;
	}

	
	void MaterialInstance::setDepthMode(EDepthMode depthMode)
	{
		mResource->mDepthMode = depthMode;
	}


	EDepthMode MaterialInstance::getDepthMode() const
	{
		if (mResource->mDepthMode != EDepthMode::NotSet)
			return mResource->mDepthMode;

		return mResource->mMaterial->getDepthMode();
	}


	//////////////////////////////////////////////////////////////////////////
	// Material
	//////////////////////////////////////////////////////////////////////////

	bool Material::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mShader != nullptr, "Shader not set in material %s", mID.c_str()))
			return false;

		const opengl::UniformDeclarations& uniform_declarations = mShader->getShader().getUniformDeclarations();

		// Here we are going to create a set of uniforms for all the uniforms that are present in the shader.
		// Some of those uniforms are also present in our input data (mUniforms), in that case we copy the
		// rtti attributes from those input objects. In any other case we create default uniforms.
		// It is important to note why we always create new uniforms and do not touch the input data:
		// - Because we create default uniforms with default values, the ownership of those newly created objects
		//   lies within the Material. It is very inconvenient if half of the Uniforms is owned by the system and
		//   half of the Uniforms by Material. Instead, materials owns all of them.
		// - It is important to maintain a separation between the input data (mUniforms) and the runtime data. Json objects
		//   should always be considered constant.
		// - The separation makes it easy to build faster mappings for textures and values, and to provide a map interface
		//   instead of a vector interface (which is what is supported at the moment for serialization).

		for (auto& kvp : uniform_declarations)
		{
			const std::string& name = kvp.first;
			const opengl::UniformDeclaration& declaration = *kvp.second;

			// See if we have a matching uniform in our input data
			Uniform* matching_uniform = nullptr;
			for (ResourcePtr<Uniform>& uniform : mUniforms)
			{
				if (uniform->mName == name)
				{
					matching_uniform = uniform.get();
					break;
				}
			}

			// Create a new uniform
			std::unique_ptr<Uniform> new_uniform = createUniformFromDeclaration(declaration);

			// If there is a match, see if the types match and copy the attributes from the input object
			if (matching_uniform != nullptr)
			{
				if (!errorState.check(matching_uniform->getGLSLType() == declaration.mGLSLType, "Uniform %s does not match the variable type in the shader %s", matching_uniform->mName.c_str(), mShader->mID.c_str()))
					return false;

				nap::rtti::copyObject(*matching_uniform, *new_uniform.get());
			}

			AddUniform(std::move(new_uniform), declaration);
		}

		return true;
	}


	const std::vector<Material::VertexAttributeBinding>& Material::sGetDefaultVertexAttributeBindings()
	{
		static std::vector<Material::VertexAttributeBinding> bindings;
		if (bindings.empty())
		{
			bindings.push_back({ VertexAttributeIDs::getPositionName(),		opengl::Shader::VertexAttributeIDs::getPositionVertexAttr() });
			bindings.push_back({ VertexAttributeIDs::getNormalName(),		opengl::Shader::VertexAttributeIDs::getNormalVertexAttr() });
			bindings.push_back({ VertexAttributeIDs::getTangentName(),		opengl::Shader::VertexAttributeIDs::getTangentVertexAttr() });
			bindings.push_back({ VertexAttributeIDs::getBitangentName(),	opengl::Shader::VertexAttributeIDs::getBitangentVertexAttr() });

			const int numChannels = 4;
			for (int channel = 0; channel != numChannels; ++channel)
			{
				bindings.push_back({ VertexAttributeIDs::GetColorName(channel), opengl::Shader::VertexAttributeIDs::getColorVertexAttr(channel) });
				bindings.push_back({ VertexAttributeIDs::getUVName(channel),	opengl::Shader::VertexAttributeIDs::getUVVertexAttr(channel) });
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

	const Material::VertexAttributeBinding* Material::findVertexAttributeBinding(const std::string& shaderAttributeID) const
	{
		// If no bindings are specified at all, use the default bindings. Note that we don't just initialize mVertexAttributeBindings to the default in init(),
		// because that would modify this object, which would cause the object diff during real-time editing to flag this object as 'modified', even though it's not.
		const std::vector<VertexAttributeBinding>& bindings = !mVertexAttributeBindings.empty() ? mVertexAttributeBindings : sGetDefaultVertexAttributeBindings();
		for (const VertexAttributeBinding& binding : bindings)
		{
			if (binding.mShaderAttributeID == shaderAttributeID)
			{
				return &binding;
			}
		}
		return nullptr;
	}
}