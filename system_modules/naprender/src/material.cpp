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

RTTI_BEGIN_STRUCT(nap::Material::VertexAttributeBinding, "Mesh (CPU) to shader (GPU) vertex binding")
	RTTI_VALUE_CONSTRUCTOR(const std::string&, const std::string&)
	RTTI_PROPERTY("MeshAttributeID",			&nap::Material::VertexAttributeBinding::mMeshAttributeID, nap::rtti::EPropertyMetaData::Required, "Mesh vertex attribute name, ie: 'Position'")
	RTTI_PROPERTY("ShaderAttributeID",			&nap::Material::VertexAttributeBinding::mShaderAttributeID, nap::rtti::EPropertyMetaData::Required, "Shader vertex input name, ie: 'in_Position")
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseMaterial, "GPU program interface")
	RTTI_PROPERTY(nap::material::uniforms,		&nap::BaseMaterial::mUniforms,				nap::rtti::EPropertyMetaData::Embedded, "Uniform inputs, binds numeric data (structs)")
	RTTI_PROPERTY(nap::material::samplers,		&nap::BaseMaterial::mSamplers,				nap::rtti::EPropertyMetaData::Embedded, "Sampler inputs, binds textures")
	RTTI_PROPERTY(nap::material::buffers,		&nap::BaseMaterial::mBuffers,				nap::rtti::EPropertyMetaData::Embedded, "Buffer inputs, binds large containers")
	RTTI_PROPERTY(nap::material::constants,		&nap::BaseMaterial::mConstants,				nap::rtti::EPropertyMetaData::Embedded, "Shader specialization constants")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Material, "Graphics shader interface, binds data to a GPU graphics program")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY(nap::material::shader,		&nap::Material::mShader,					nap::rtti::EPropertyMetaData::Required,	"The GPU graphics program")
	RTTI_PROPERTY(nap::material::vbindings,		&nap::Material::mVertexAttributeBindings,	nap::rtti::EPropertyMetaData::Default,	"Optional vertex mapping, from mesh (CPU) vertex attribute to shader (GPU) vertex attribute")
	RTTI_PROPERTY("BlendMode",					&nap::Material::mBlendMode,					nap::rtti::EPropertyMetaData::Default,	"Default color blend mode")
	RTTI_PROPERTY("DepthMode",					&nap::Material::mDepthMode,					nap::rtti::EPropertyMetaData::Default,	"Default depth mode")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComputeMaterial, "Compute shader interface, binds data to a GPU compute program")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY(nap::material::shader,		&nap::ComputeMaterial::mShader,				nap::rtti::EPropertyMetaData::Required, "The GPU compute program")
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// BaseMaterial
	//////////////////////////////////////////////////////////////////////////

	BaseMaterial::BaseMaterial(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }

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
		// Store shader
		mShader = &shader;

		// Uniforms
		const auto& ubo_declarations = shader.getUBODeclarations();
		for (const BufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			const auto* struct_resource = rtti_cast<const UniformStruct>(findUniformStructMember(mUniforms, ubo_declaration));

			UniformStructInstance& root_struct = createUniformRootStruct(ubo_declaration, UniformCreatedCallback());
			if (!root_struct.addUniformRecursive(ubo_declaration, struct_resource, UniformCreatedCallback(), true, errorState))
				return false;
		}

		// Bindings
		const auto& ssbo_declarations = shader.getSSBODeclarations();
		for (const auto& declaration : ssbo_declarations)
		{
			std::unique_ptr<BufferBindingInstance> binding_instance;
			for (auto& binding : mBuffers)
			{
				const std::string& binding_name = declaration.mName;
				if (binding_name == binding->mName)
				{
					// Create a buffer binding instance of the appropriate type
					binding_instance = BufferBindingInstance::createBufferBindingInstanceFromDeclaration(declaration, binding.get(), BufferBindingChangedCallback(), errorState);
					if (!errorState.check(binding_instance != nullptr, "Failed to create buffer binding instance %s", binding->mName.c_str()))
						return false;

					addBindingInstance(std::move(binding_instance));
					break;
				}
			}
		}

		// Samplers
		const auto& sampler_declarations = shader.getSamplerDeclarations();
		for (const auto& declaration : sampler_declarations)
		{
			if (!errorState.check(declaration.mType == SamplerDeclaration::EType::Type_2D || declaration.mType == SamplerDeclaration::EType::Type_Cube, "Only 2D or Cube samplers are currently supported"))
				return false;

			std::unique_ptr<SamplerInstance> sampler_instance;
			for (auto& sampler : mSamplers)
			{
				if (sampler->mName == declaration.mName)
				{
					bool target_is_array = sampler->get_type().is_derived_from<SamplerArray>();

					if (!errorState.check(declaration.mIsArray == target_is_array, "Sampler '%s' does not match array type of sampler in shader", sampler->mName.c_str()))
						return false;

					if (declaration.mIsArray)
                        sampler_instance = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, (Sampler2DArray*)sampler.get(), SamplerChangedCallback());
					else
						sampler_instance = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, (Sampler2D*)sampler.get(), SamplerChangedCallback());
				}
			}

			if (sampler_instance == nullptr)
			{
				if (declaration.mIsArray)
                    sampler_instance = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, nullptr, SamplerChangedCallback());
                else
					sampler_instance = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, nullptr, SamplerChangedCallback());
			}

			if (!sampler_instance->init(errorState))
				return false;

			addSamplerInstance(std::move(sampler_instance));
		}

		// Constants
		const auto& constant_declarations = shader.getConstantDeclarations();
		for (const auto& declaration : constant_declarations)
		{
			std::unique_ptr<ShaderConstantInstance> constant_instance;
			for (const auto& constant : mConstants)
			{
				if (constant->mName == declaration.mName)
				{
					constant_instance = std::make_unique<ShaderConstantInstance>(declaration, constant.get());
					break;
				}
			}
			if (constant_instance == nullptr)
				constant_instance = std::make_unique<ShaderConstantInstance>(declaration, nullptr);

			addConstantInstance(std::move(constant_instance));
		}

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Material
	//////////////////////////////////////////////////////////////////////////

	Material::Material(Core& core) :
		BaseMaterial(core)
	{ }


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

		if (!rebuild(*mShader, errorState))
			return false;

		return true;
	}


	glm::uvec3 ComputeMaterial::getWorkGroupSize() const
	{
		// Override default values of workgroup constants
		const auto& override_map = getShader().getWorkGroupSizeOverrides();
		glm::uvec3 workgroup_size = getShader().getWorkGroupSize();
		for (const auto& entry : override_map)
		{
			assert(entry.first <= workgroup_size.length());
			auto* constant = findConstant(entry.second);
			if (constant != nullptr)
				workgroup_size[entry.first] = constant->mValue;
		}
		return workgroup_size;
	}
}
