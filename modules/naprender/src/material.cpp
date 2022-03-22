/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "material.h"
#include "mesh.h"
#include "renderglobals.h"
#include "renderservice.h"
#include "vk_mem_alloc.h"

// External includes
#include <nap/logger.h>
#include <rtti/rttiutilities.h>
#include <nap/core.h>

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


RTTI_DEFINE_BASE(nap::BaseMaterial)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Material)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Uniforms",					&nap::Material::mUniforms,					nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Bindings",					&nap::Material::mBufferBindings,			nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Samplers",					&nap::Material::mSamplers,					nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Shader",						&nap::Material::mShader,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("VertexAttributeBindings",	&nap::Material::mVertexAttributeBindings,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendMode",					&nap::Material::mBlendMode,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",					&nap::Material::mDepthMode,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeMaterial)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Uniforms",					&nap::ComputeMaterial::mUniforms,			nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Bindings",					&nap::ComputeMaterial::mBufferBindings,		nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Samplers",					&nap::ComputeMaterial::mSamplers,			nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Shader",						&nap::ComputeMaterial::mShader,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// BaseMaterial
	//////////////////////////////////////////////////////////////////////////

	BaseMaterial::BaseMaterial(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}

	/**
	 * The BaseMaterial rebuild will initialize all uniforms that can be used with the bound shader. The shader contains the authoritative set of Uniforms that can be set;
	 * the Uniforms defined in the material must match the Uniforms declared by the shader. If the shader declares a Uniform that is not present in the Material, a
	 * default Uniform will be used.
	 *
	 * To prevent us from having to check everywhere whether a uniform is present in the material or not, we create Uniforms for *all* uniforms
	 * declared by the shader, even if they're present in the material. Then, if the Uniform is also present in the material, we simply copy the existing uniform over the
	 * newly-constructed uniform. Furthermore, it is important to note why we always create new uniforms and do not touch the input data:
	 *
	 * - Because we create default uniforms with default values, the ownership of those newly created objects
	 *   lies within the Material. It is very inconvenient if half of the Uniforms is owned by the system and
	 *   half of the Uniforms by Material. Instead, Material owns all of them.
	 * - It is important to maintain a separation between the input data (mUniforms) and the runtime data. Json objects
	 *   should always be considered constant.
	 * - The separation makes it easy to build faster mappings for textures and values, and to provide a map interface
	 *   instead of a vector interface (which is what is supported at the moment for serialization).
	 *
	 * Thus, the Material init happens in two passes:
	 * - First, we create uniforms for all uniforms declared in the shader. This is a recursive process, due to the presence of arrays/struct uniforms
	 * - Then, we apply all uniforms that are present in the material (mUniforms) onto the newly-constructed uniforms. This is also a recursive process.
	 *
	 * Note that the first pass creates a 'tree' of uniforms (arrays can contain structs, which can contains uniforms, etc); the tree of uniforms defined in the material
	 * must match the tree generated in the first pass.
	 */
	bool BaseMaterial::rebuild(const BaseShader& shader, utility::ErrorState& errorState)
	{
		// Uniforms
		const std::vector<BufferObjectDeclaration>& ubo_declarations = shader.getUBODeclarations();
		for (const BufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			const UniformStruct* struct_resource = rtti_cast<const UniformStruct>(findUniformStructMember(mUniforms, ubo_declaration));

			UniformStructInstance& root_struct = createUniformRootStruct(ubo_declaration, UniformCreatedCallback());
			if (!root_struct.addUniformRecursive(ubo_declaration, struct_resource, UniformCreatedCallback(), true, errorState))
				return false;
		}

		// Bindings
		const std::vector<BufferObjectDeclaration>& ssbo_declarations = shader.getSSBODeclarations();
		for (const BufferObjectDeclaration& declaration : ssbo_declarations)
		{
			std::unique_ptr<BufferBindingInstance> binding_instance;
			for (auto& binding : mBufferBindings)
			{
				if (binding->mName == declaration.mName)
				{
					// We must check if the SSBO declaration contains more than a single shader variable and exit early if this is the case.
					// The reason for this is that we want to associate a shader buffer resource binding point with single shader storage
					// buffer (VkBuffer), this is a typical use case for storage buffers and simplifies overall resource management. At the
					// same time we use regular shader variable declarations, that assume a list of member variables, to generate buffer bindings.
					if (!errorState.check(declaration.mMembers.size() <= 1, utility::stringFormat("SSBO '%s' contains more than 1 shader variable, which is currently not supported. Consider using multiple SSBO's or a struct array.", declaration.mName.c_str())))
						return false;

					// Get the first and only member of the declaration
					const auto& buffer_declaration = declaration.getBufferDeclaration();

					// Create a buffer binding instance of the appropriate type
					binding_instance = BufferBindingInstance::createBufferBindingInstanceFromDeclaration(buffer_declaration, binding.get(), BufferBindingChangedCallback(), errorState);
					if (!errorState.check(binding_instance != nullptr, "Failed to create buffer binding instance %s", binding->mName.c_str()))
						return false;

					addBufferBindingInstance(std::move(binding_instance));
				}
			}
		}

		// Samplers
		const SamplerDeclarations& sampler_declarations = shader.getSamplerDeclarations();
		for (const SamplerDeclaration& declaration : sampler_declarations)
		{
			if (!errorState.check(declaration.mType == SamplerDeclaration::EType::Type_2D, "Non-2D samplers are not supported"))
				return false;

			bool is_array = declaration.mNumArrayElements > 1;
			std::unique_ptr<SamplerInstance> sampler_instance;
			for (auto& sampler : mSamplers)
			{
				if (sampler->mName == declaration.mName)
				{
					bool target_is_array = sampler->get_type().is_derived_from<SamplerArray>();

					if (!errorState.check(is_array == target_is_array, "Sampler '%s' does not match array type of sampler in shader", sampler->mName.c_str()))
						return false;

					if (is_array)
						sampler_instance = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, (Sampler2DArray*)sampler.get(), SamplerChangedCallback());
					else
						sampler_instance = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, (Sampler2D*)sampler.get(), SamplerChangedCallback());
				}
			}

			if (sampler_instance == nullptr)
			{
				if (is_array)
					sampler_instance = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, nullptr, SamplerChangedCallback());
				else
					sampler_instance = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, nullptr, SamplerChangedCallback());
			}

			if (!sampler_instance->init(errorState))
				return false;

			addSamplerInstance(std::move(sampler_instance));
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Material
	//////////////////////////////////////////////////////////////////////////

	Material::Material(Core& core) :
		BaseMaterial(core)
	{
	}


	bool Material::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mShader != nullptr, "Shader not set in material %s", mID.c_str()))
			return false;

		return rebuild(*mShader, errorState);
	}


	const std::vector<Material::VertexAttributeBinding>& Material::sGetDefaultVertexAttributeBindings()
	{
		static std::vector<Material::VertexAttributeBinding> bindings;
		if (bindings.empty())
		{
			bindings.push_back({ vertexid::position,	vertexid::shader::position });
			bindings.push_back({ vertexid::normal,		vertexid::shader::normal });
			bindings.push_back({ vertexid::tangent,		vertexid::shader::tangent });
			bindings.push_back({ vertexid::bitangent,	vertexid::shader::bitangent });

			const int numChannels = 4;
			for (int channel = 0; channel != numChannels; ++channel)
			{
				bindings.push_back({ vertexid::getColorName(channel), vertexid::shader::getColorInputName(channel) });
				bindings.push_back({ vertexid::getUVName(channel), vertexid::shader::getUVInputName(channel) });
			}
		}
		return bindings;
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


	Material::VertexAttributeBinding::VertexAttributeBinding(const std::string& meshAttributeID, const std::string& shaderAttributeID) :
		mMeshAttributeID(meshAttributeID),
		mShaderAttributeID(shaderAttributeID)
	{ }


	//////////////////////////////////////////////////////////////////////////
	// ComputeMaterial
	//////////////////////////////////////////////////////////////////////////

	ComputeMaterial::ComputeMaterial(Core& core) :
		BaseMaterial(core)
	{
	}

	bool ComputeMaterial::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mShader != nullptr, "Shader not set in material %s", mID.c_str()))
			return false;

		return rebuild(*mShader, errorState);
	}
}
