// Local Includes
#include "material.h"
#include "nshader.h"

// External includes
#include <nap/logger.h>
#include <GL/glew.h>
#include "rtti/rttiutilities.h"


RTTI_BEGIN_CLASS(nap::Material::VertexAttributeBinding)
	RTTI_PROPERTY("MeshAttributeID",	&nap::Material::VertexAttributeBinding::mMeshAttributeID,	RTTI::EPropertyMetaData::Required)
	RTTI_PROPERTY("ShaderAttributeID",	&nap::Material::VertexAttributeBinding::mShaderAttributeID,	RTTI::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Material)
	RTTI_PROPERTY("Shader",						&nap::Material::mShader,					RTTI::EPropertyMetaData::Required)
	RTTI_PROPERTY("VertexAttributeBindings",	&nap::Material::mVertexAttributeBindings,	RTTI::EPropertyMetaData::Default)
	RTTI_PROPERTY("Uniforms",					&nap::Material::mUniforms,					RTTI::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{
	std::unique_ptr<Uniform> createUniform(const opengl::UniformDeclaration& declaration)
	{
		std::unique_ptr<Uniform> result;

		switch (declaration.mGLSLType)
		{
		case opengl::GLSLType::Int:
			result = std::make_unique<UniformInt>();
			break;
		case opengl::GLSLType::Vec4:
			result = std::make_unique<UniformVec4>();
			break;
		case opengl::GLSLType::Mat4:
			result = std::make_unique<UniformMat4>();
			break;
		case opengl::GLSLType::Tex2D:
			result = std::make_unique<UniformTexture2D>();
			break;
		}
		assert(result);
		result->mName = declaration.mName;
		return result;
	}


	bool Material::init(ErrorState& errorState)
	{
		if (!errorState.check(mShader != nullptr, "Shader not set in material"))
			return false;

		// Store state for rollback
		mPrevUniformTextureBindings = std::move(mUniformTextureBindings);
		mPrevUniformValueBindings = std::move(mUniformValueBindings);

		// Start empty
		mUniformTextureBindings.clear();
		mUniformValueBindings.clear();

		const opengl::UniformDeclarations& uniform_declarations = mShader->getShader().getUniformDeclarations();

		// Here we are going to create a set of uniforms for all the uniforms that are present in the shader.
		// Some of those uniforms are also present in our input data (mUniforms), in that case we copy the
		// rtti attributes from those input objects. In any other case we create default uniforms.
		// It is important to note why we always create new uniforms and do not touch the input data:
		// - Because we create default uniforms with default values, the ownership of those newly created objects
		//   lies within the Material. It is very inconvenient if half of the Uniforms is owned by the system and
		//   half of the Uniforms by Material. Instead, materials owns all of them.
		// - It is important to maintain a separation between the input data (mUniforms) and the runtime data. For real-time
		//   editing we can now just store runtime data for a rollback without any reference to the original input data
		//   that can get modified during updating.
		// - The separation makes it easy to build faster mappings for textures and values, and to provide a map interface
		//   instead of a vector interface (which is what is supported at the moment for serialization).

		for (auto& kvp : uniform_declarations)
		{
			const std::string& name = kvp.first;
			const opengl::UniformDeclaration& declaration = *kvp.second;

			// See if we have a matching uniform in our input data
			Uniform* matching_uniform = nullptr;
			for (Uniform* uniform : mUniforms)
			{
				if (uniform->mName == name)
				{
					matching_uniform = uniform;
					break;
				}
			}

			// Create a new uniform
			std::unique_ptr<Uniform> new_uniform = createUniform(declaration);

			// If there is a match, see if the types match and copy the attributes from the input object
			if (matching_uniform != nullptr)
			{
				if (!errorState.check(matching_uniform->getGLSLType() == declaration.mGLSLType, "Uniform %s does not match the variable type in the shader"))
					return false;

				RTTI::copyObject(*matching_uniform, *new_uniform.get());
			}

			// Create association between uniform and declaration. At the same time, split between textures and values
			// as texture have a slightly different interface.
			std::unique_ptr<UniformTexture> texture_uniform = rtti_cast<UniformTexture>(new_uniform);
			if (texture_uniform == nullptr)
			{
				std::unique_ptr<UniformValue> value_uniform = rtti_cast<UniformValue>(new_uniform);
				assert(value_uniform);
				mUniformValueBindings.emplace(std::make_pair(name, UniformBinding<UniformValue>(std::move(value_uniform), declaration)));
			}
			else
			{
				mUniformTextureBindings.emplace(std::make_pair(name, UniformBinding<UniformTexture>(std::move(texture_uniform), declaration)));
			}
		}
		
		// Verify that we don't have uniform mapping that do not exist in the shader
		for (const Uniform* uniform : mUniforms)
		{
			opengl::UniformDeclarations::const_iterator declaration = uniform_declarations.find(uniform->mName);
			if (!errorState.check(declaration != uniform_declarations.end(), "Unable to find uniform %s in shader", uniform->mName))
				return false;
		}

		return true;
	}


	void Material::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			mPrevUniformTextureBindings.clear();
			mPrevUniformValueBindings.clear();
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			mUniformTextureBindings = std::move(mPrevUniformTextureBindings);
			mUniformValueBindings = std::move(mPrevUniformValueBindings);
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
	void Material::pushUniforms()
	{
		// Push values
		for (auto& kvp : mUniformValueBindings)
			kvp.second.mUniform->push(*kvp.second.mDeclaration);

		// Push textures
		int texture_unit = 0;
		for (auto& kvp : mUniformTextureBindings)
			kvp.second.mUniform->push(*kvp.second.mDeclaration, texture_unit++);

		glActiveTexture(GL_TEXTURE0);
	}


	const Material::VertexAttributeBinding* Material::findVertexAttributeBinding(const opengl::Mesh::VertexAttributeID& shaderAttributeID) const
	{
		for (const VertexAttributeBinding& binding : mVertexAttributeBindings)
			if (binding.mShaderAttributeID == shaderAttributeID)
				return &binding;

		return nullptr;
	}
}