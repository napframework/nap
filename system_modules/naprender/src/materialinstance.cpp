/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "materialinstance.h"
#include "mesh.h"
#include "material.h"
#include "renderservice.h"
#include "vk_mem_alloc.h"
#include "renderutils.h"
#include "imagedata.h"

// External includes
#include <nap/logger.h>
#include <rtti/rttiutilities.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseMaterialInstanceResource, "GPU program overrides")
	RTTI_PROPERTY(nap::material::uniforms,		&nap::MaterialInstanceResource::mUniforms,					nap::rtti::EPropertyMetaData::Embedded, "Uniform inputs, overrides and binds numeric data (structs)")
	RTTI_PROPERTY(nap::material::samplers,		&nap::MaterialInstanceResource::mSamplers,					nap::rtti::EPropertyMetaData::Embedded, "Sampler inputs, overrides and binds textures")
	RTTI_PROPERTY(nap::material::buffers,		&nap::MaterialInstanceResource::mBuffers,					nap::rtti::EPropertyMetaData::Embedded, "Buffer inputs, overrides and binds large containers")
	RTTI_PROPERTY(nap::material::constants,		&nap::MaterialInstanceResource::mConstants,					nap::rtti::EPropertyMetaData::Embedded, "Shader specialization constant overrides")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MaterialInstanceResource, "Applies a graphics material and provides an interface to override input defaults")
	RTTI_PROPERTY(nap::MaterialInstanceResource::matProperty,	&nap::MaterialInstanceResource::mMaterial,	nap::rtti::EPropertyMetaData::Required, "Graphics material default")
	RTTI_PROPERTY("BlendMode",	&nap::MaterialInstanceResource::mBlendMode,		nap::rtti::EPropertyMetaData::Default, "Color blend mode override")
	RTTI_PROPERTY("DepthMode",	&nap::MaterialInstanceResource::mDepthMode,		nap::rtti::EPropertyMetaData::Default, "Depth mode override")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ComputeMaterialInstanceResource, "Applies a compute material and provides an interface to override input defaults")
	RTTI_PROPERTY(nap::ComputeMaterialInstanceResource::matProperty, &nap::ComputeMaterialInstanceResource::mComputeMaterial,	nap::rtti::EPropertyMetaData::Required, "Compute material default")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseMaterialInstance)
	RTTI_FUNCTION("getOrCreateUniform", (nap::UniformStructInstance* (nap::BaseMaterialInstance::*)(const std::string&))& nap::BaseMaterialInstance::getOrCreateUniform);
	RTTI_FUNCTION("getOrCreateSampler", (nap::SamplerInstance* (nap::BaseMaterialInstance::*)(const std::string&))& nap::BaseMaterialInstance::getOrCreateSampler);
RTTI_END_CLASS

RTTI_DEFINE_CLASS(nap::MaterialInstance)
RTTI_DEFINE_CLASS(nap::ComputeMaterialInstance)

namespace nap
{
	template<class T>
	const UniformInstance* findUniformStructInstanceMember(const std::vector<T>& members, const std::string& name)
	{
		for (auto& member : members)
			if (member->getDeclaration().mName == name)
				return member.get();

		return nullptr;
	}


	template<class T>
	const BufferBinding* findBindingResource(const std::vector<T>& bindings, const BufferObjectDeclaration& declaration)
	{
		for (auto& binding : bindings)
			if (binding->mName == declaration.mName)
				return binding.get();

		return nullptr;
	}


	template<class T>
	const Sampler* findSamplerResource(const std::vector<T>& samplers, const SamplerDeclaration& declaration)
	{
		for (auto& sampler : samplers)
			if (sampler->mName == declaration.mName)
				return sampler.get();

		return nullptr;
	}


	template<class T>
	const ShaderConstant* findConstantResource(const std::vector<T>& constants, const ShaderConstantDeclaration& declaration)
	{
		for (auto& constant : constants)
			if (constant->mName == declaration.mName)
				return constant.get();

		return nullptr;
	}


	void buildUniformBufferObjectRecursive(const UniformStructInstance& baseInstance, const UniformStructInstance* overrideInstance, UniformBufferObject& ubo)
	{
		for (auto& base_uniform : baseInstance.getUniforms())
		{
			rtti::TypeInfo declaration_type = base_uniform->get_type();

			const UniformInstance* override_uniform = nullptr;
			if (overrideInstance != nullptr)
				override_uniform = findUniformStructInstanceMember(overrideInstance->getUniforms(), base_uniform->getDeclaration().mName);

			if (declaration_type == RTTI_OF(UniformStructArrayInstance))
			{
				const UniformStructArrayInstance* struct_array_override = rtti_cast<const UniformStructArrayInstance>(override_uniform);
				const UniformStructArrayInstance* struct_array_declaration = rtti_cast<const UniformStructArrayInstance>(base_uniform.get());

				int resource_index = 0;
				for (auto& base_element : struct_array_declaration->getElements())
				{
					UniformStructInstance* element_override = nullptr;
					if (struct_array_override != nullptr && resource_index < struct_array_override->getElements().size())
						element_override = struct_array_override->getElements()[resource_index++].get();

					buildUniformBufferObjectRecursive(*base_element, element_override, ubo);
				}
			}
			else if (declaration_type.is_derived_from(RTTI_OF(UniformValueArrayInstance)))
			{
				const UniformValueArrayInstance* base_array_uniform = rtti_cast<const UniformValueArrayInstance>(base_uniform.get());
				const UniformValueArrayInstance* override_array_uniform = rtti_cast<const UniformValueArrayInstance>(override_uniform);

				if (override_array_uniform != nullptr)
					ubo.mUniforms.push_back(override_array_uniform);
				else
					ubo.mUniforms.push_back(base_array_uniform);
			}
			else if (declaration_type == RTTI_OF(UniformStructInstance))
			{
				const UniformStructInstance* base_struct = rtti_cast<const UniformStructInstance>(base_uniform.get());
				const UniformStructInstance* override_struct = rtti_cast<const UniformStructInstance>(override_uniform);

				buildUniformBufferObjectRecursive(*base_struct, override_struct, ubo);
			}
			else
			{
				const UniformValueInstance* base_value = rtti_cast<const UniformValueInstance>(base_uniform.get());
				const UniformValueInstance* override_value = rtti_cast<const UniformValueInstance>(override_uniform);

				if (override_value != nullptr)
					ubo.mUniforms.push_back(override_value);
				else
					ubo.mUniforms.push_back(base_value);
			}
		}
	}


	void updateUniforms(const DescriptorSet& descriptorSet, std::vector<UniformBufferObject>& bufferObjects)
	{
		// Go over all the UBOs and memcpy the latest MaterialInstance state into the allocated descriptorSet's VkBuffers
		for (int ubo_index = 0; ubo_index != descriptorSet.mBuffers.size(); ++ubo_index)
		{
			UniformBufferObject& ubo = bufferObjects[ubo_index];
			VmaAllocationInfo allocation = descriptorSet.mBuffers[ubo_index].mAllocationInfo;

			void* mapped_memory = allocation.pMappedData;
			for (auto& uniform : ubo.mUniforms)
			{
				uniform->push((uint8_t*)mapped_memory);
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// BaseMaterialInstance
	//////////////////////////////////////////////////////////////////////////


	BaseMaterialInstanceResource::BaseMaterialInstanceResource(std::string&& materialPropertyName) :
		mMaterialPropertyName(std::move(materialPropertyName))
	{ }


	rttr::property BaseMaterialInstanceResource::getMaterialProperty() const
	{
		auto prop = get_type().get_property(mMaterialPropertyName.data());
		assert(prop.is_valid());
		return prop;
	}


	const std::string& BaseMaterialInstanceResource::getMaterialPropertyName() const
	{
		return mMaterialPropertyName;
	}


	UniformStructInstance* BaseMaterialInstance::getOrCreateUniform(const std::string& name)
	{
		UniformStructInstance* existing = findUniform(name);
		if (existing != nullptr)
			return existing;

		// Find the declaration in the shader (if we can't find it, it's not a name that actually exists in the shader, which is an error).
		const std::vector<BufferObjectDeclaration>& ubo_declarations = getMaterial()->getShader().getUBODeclarations();
		for (const BufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			if (ubo_declaration.mName == name)
			{
				// At the MaterialInstance level, we always have UBOs at the root, so we create a root struct
				return &createUniformRootStruct(ubo_declaration,
					std::bind(&BaseMaterialInstance::onUniformCreated, this));
			}
		}
		return nullptr;
	}


	BufferBindingInstance* BaseMaterialInstance::getOrCreateBufferInternal(const std::string& name)
	{
		// See if we have an override in MaterialInstance. If so, we can return it
		BufferBindingInstance* existing_binding = findBinding(name);
		if (existing_binding != nullptr)
			return existing_binding;

		BufferBindingInstance* result = nullptr;

		// Find the declaration in the shader (if we can't find it, it's not a name that actually exists in the shader, which is an error).
		const std::vector<BufferObjectDeclaration>& ssbo_declarations = getMaterial()->getShader().getSSBODeclarations();
		int ssbo_index = 0;
		for (const BufferObjectDeclaration& declaration : ssbo_declarations)
		{
			const std::string& binding_name = declaration.mName;
			if (binding_name == name)
			{
				std::unique_ptr<BufferBindingInstance> binding_instance_override;

				utility::ErrorState error_state;
				binding_instance_override = BufferBindingInstance::createBufferBindingInstanceFromDeclaration(declaration, nullptr, std::bind(&BaseMaterialInstance::onBufferChanged, this, ssbo_index, std::placeholders::_1), error_state);

				if (!error_state.check(binding_instance_override != nullptr, "Failed to create buffer binding instance"))
					NAP_ASSERT_MSG(binding_instance_override != nullptr, error_state.toString().c_str());

				result = &addBindingInstance(std::move(binding_instance_override));
				break;
			}
			++ssbo_index;
		}
		return result;
	}


	SamplerInstance* BaseMaterialInstance::getOrCreateSamplerInternal(const std::string& name, const Sampler* resource)
	{
		// See if we have an override in MaterialInstance. If so, we can return it
		SamplerInstance* existing_sampler = findSampler(name);
		if (existing_sampler != nullptr)
			return existing_sampler;

		SamplerInstance* result = nullptr;

		const BaseShader& shader = getMaterial()->getShader();
		const SamplerDeclarations& sampler_declarations = shader.getSamplerDeclarations();
		int image_start_index = 0;
		for (const SamplerDeclaration& declaration : sampler_declarations)
		{
			if (declaration.mName == name)
			{
				std::unique_ptr<SamplerInstance> sampler_instance_override;
				if (declaration.mIsArray)
				{
					switch (declaration.mType)
					{
					case SamplerDeclaration::EType::Type_2D:
					{
						assert(resource == nullptr || resource->get_type().is_derived_from(RTTI_OF(Sampler2DArray)));
						const auto* sampler_2d_array = static_cast<const Sampler2DArray*>(resource);
						sampler_instance_override = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, sampler_2d_array,
							std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1, std::placeholders::_2));
						break;
					}

					case SamplerDeclaration::EType::Type_Cube:
					{
						assert(resource == nullptr || resource->get_type().is_derived_from(RTTI_OF(SamplerCubeArray)));
						const auto* sampler_cube_array = static_cast<const SamplerCubeArray*>(resource);
						sampler_instance_override = std::make_unique<SamplerCubeArrayInstance>(*mRenderService, declaration, sampler_cube_array,
							std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1, std::placeholders::_2));
						break;
					}
					default:
						NAP_ASSERT_MSG(false, "Unsupported sampler declaration type");
					}
				}
				else
				{
					switch (declaration.mType)
					{
					case SamplerDeclaration::EType::Type_2D:
					{
						assert(resource == nullptr || resource->get_type().is_derived_from(RTTI_OF(Sampler2D)));
						const auto* sampler_2d = static_cast<const Sampler2D*>(resource);
						sampler_instance_override = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, sampler_2d,
							std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1, std::placeholders::_2));
						break;
					}
					case SamplerDeclaration::EType::Type_Cube:
					{
						assert(resource == nullptr || resource->get_type().is_derived_from(RTTI_OF(SamplerCube)));
						const auto* sampler_cube = static_cast<const SamplerCube*>(resource);
						sampler_instance_override = std::make_unique<SamplerCubeInstance>(*mRenderService, declaration, sampler_cube,
							std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1, std::placeholders::_2));
						break;
					}
					default:
						NAP_ASSERT_MSG(false, "Unsupported sampler declaration type");
					}
				}

				utility::ErrorState error_state;
				bool initialized = sampler_instance_override->init(error_state); assert(initialized);
				result = &addSamplerInstance(std::move(sampler_instance_override));
				break;
			}
			image_start_index += declaration.mNumElements;
		}
		return result;
	}


	void BaseMaterialInstance::onUniformCreated()
	{
		// We only store that uniforms have been created. During update() we will update UBO structures. The reason
		// why we don't do this in place is because we to avoid multiple rebuilds for a single draw.
		mUniformsCreated = true;
	}


	// This function is called whenever a SamplerInstance changes its texture. We already have VkWriteDescriptorSets
	// that contain image information that point into the mSampleImages array. The information in VkWriteDescriptorSets
	// remains static after init, no matter what textures we use, because it is pointing to indices into the mSamplerImages array.
	// What we need to do here is update the contents of the mSamplerImages array so that it points to correct information
	// for the texture change. This way, when update() is called, VkUpdateDescriptorSets will use the correct image info.
	void BaseMaterialInstance::onSamplerChanged(int imageStartIndex, SamplerInstance& samplerInstance, int imageArrayIndex)
	{
		VkSampler vk_sampler = samplerInstance.getVulkanSampler();

		if (samplerInstance.get_type().is_derived_from(RTTI_OF(SamplerArrayInstance)))
		{
			int sampler_descriptor_index = imageStartIndex + imageArrayIndex;
			if (mSamplerDescriptors.size() < sampler_descriptor_index)
				mSamplerDescriptors.emplace_back();

			VkDescriptorImageInfo& image_info = mSamplerDescriptors[sampler_descriptor_index];
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.sampler = vk_sampler;

			if (samplerInstance.get_type() == RTTI_OF(Sampler2DArrayInstance))
			{
				Sampler2DArrayInstance* sampler_2d_array = (Sampler2DArrayInstance*)(&samplerInstance);
				assert(imageArrayIndex < sampler_2d_array->getNumElements());

				const Texture2D& texture = sampler_2d_array->getTexture(imageArrayIndex);
				image_info.imageView = texture.getHandle().getView();

			}
			else if (samplerInstance.get_type() == RTTI_OF(SamplerCubeArrayInstance))
			{
				SamplerCubeArrayInstance* sampler_cube_array = (SamplerCubeArrayInstance*)(&samplerInstance);
				assert(imageArrayIndex < sampler_cube_array->getNumElements());

				const TextureCube& texture = sampler_cube_array->getTexture(imageArrayIndex);
				image_info.imageView = texture.getHandle().getView();
			}
			else
			{
				NAP_ASSERT_MSG(false, "Unsupported sampler type");
			}
		}
		else
		{
			VkDescriptorImageInfo& image_info = mSamplerDescriptors[imageStartIndex];
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.sampler = vk_sampler;

			if (samplerInstance.get_type() == RTTI_OF(Sampler2DInstance))
			{
				Sampler2DInstance* sampler_2d = (Sampler2DInstance*)(&samplerInstance);
				image_info.imageView = sampler_2d->getTexture().getHandle().getView();
			}
			else if (samplerInstance.get_type() == RTTI_OF(SamplerCubeInstance))
			{
				SamplerCubeInstance* sampler_cube = (SamplerCubeInstance*)(&samplerInstance);
				image_info.imageView = sampler_cube->getTexture().getHandle().getView();
			}
			else
			{
				NAP_ASSERT_MSG(false, "Unsupported sampler type");
			}
		}
	}


	void BaseMaterialInstance::onBufferChanged(int storageBufferIndex, BufferBindingInstance& bindingInstance)
	{
		// Update the buffer info structure stored in the buffer info handles
		VkDescriptorBufferInfo& buffer_info = mStorageDescriptors[storageBufferIndex];
		if (bindingInstance.get_type().is_derived_from(RTTI_OF(BufferBindingStructInstance)))
		{
			BufferBindingStructInstance* instance = static_cast<BufferBindingStructInstance*>(&bindingInstance);
			buffer_info.buffer = instance->getBuffer().getBuffer();
		}
		else if (bindingInstance.get_type().is_derived_from(RTTI_OF(BufferBindingNumericInstance)))
		{
			if (bindingInstance.get_type() == RTTI_OF(BufferBindingUIntInstance))
			{
				auto* instance = static_cast<BufferBindingUIntInstance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingIntInstance))
			{
				auto* instance = static_cast<BufferBindingIntInstance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingFloatInstance))
			{
				auto* instance = static_cast<BufferBindingFloatInstance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingVec2Instance))
			{
				auto* instance = static_cast<BufferBindingVec2Instance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingVec3Instance))
			{
				auto* instance = static_cast<BufferBindingVec3Instance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingVec4Instance))
			{
				auto* instance = static_cast<BufferBindingVec4Instance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingIVec4Instance))
			{
				auto* instance = static_cast<BufferBindingIVec4Instance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingUVec4Instance))
			{
				auto* instance = static_cast<BufferBindingUVec4Instance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
			else if (bindingInstance.get_type() == RTTI_OF(BufferBindingMat4Instance))
			{
				auto* instance = static_cast<BufferBindingMat4Instance*>(&bindingInstance);
				buffer_info.buffer = instance->getBuffer().getBuffer();
			}
		}
		else
		{
			NAP_ASSERT_MSG(false, "Unsupported buffer binding type");
		}
	}


	void BaseMaterialInstance::rebuildUBO(UniformBufferObject& ubo, UniformStructInstance* overrideStruct)
	{
		ubo.mUniforms.clear();

		const UniformStructInstance* base_struct = rtti_cast<const UniformStructInstance>(getMaterial()->findUniform(ubo.mDeclaration->mName));
		assert(base_struct != nullptr);

		buildUniformBufferObjectRecursive(*base_struct, overrideStruct, ubo);
	}


	void BaseMaterialInstance::addImageInfo(const Texture& texture, VkSampler sampler)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture.getHandle().getView();
		imageInfo.sampler = sampler;

		mSamplerDescriptors.push_back(imageInfo);
	}


	bool BaseMaterialInstance::initBuffers(BaseMaterialInstanceResource& instanceResource, utility::ErrorState& errorState)
	{
		// Here we create SSBOs in the same way as we did for UBOs above
		const auto& ssbo_declarations = getMaterial()->getShader().getSSBODeclarations();
		mStorageDescriptors.resize(ssbo_declarations.size());
		mStorageWriteDescriptorSets.reserve(ssbo_declarations.size()); // We reserve to ensure that pointers remain consistent during the iteration

		int ssbo_index = 0;
		for (const BufferObjectDeclaration& declaration : ssbo_declarations)
		{
			// Check if the binding is set as override in the MaterialInstance
			const BufferBinding* override_resource = findBindingResource(instanceResource.mBuffers, declaration);

			BufferBindingInstance* binding = nullptr;
			if (override_resource != nullptr)
			{
				// Buffer binding is overridden, make a BufferBindingInstance object
				auto override_instance = BufferBindingInstance::createBufferBindingInstanceFromDeclaration(declaration, override_resource, std::bind(&BaseMaterialInstance::onBufferChanged, this, ssbo_index, std::placeholders::_1), errorState);
				if (!errorState.check(override_instance != nullptr, "Failed to create buffer binding instance for shader variable `%s`", declaration.mName.c_str()))
					return false;

				// A buffer is required to be assigned at this point
				if (!errorState.check(override_instance->hasBuffer(), utility::stringFormat("No valid buffer was assigned to shader variable '%s' in material override '%s'", declaration.mName.c_str(), getMaterial()->mID.c_str()).c_str()))
					return false;

				binding = &addBindingInstance(std::move(override_instance));
			}
			else
			{
				// Binding is not overridden, find it in the base material
				binding = getMaterial()->findBinding(declaration.mName);
				if (!errorState.check(binding != nullptr, "Failed to find buffer binding instance for shader variable `%s` in base material", declaration.mName.c_str()))
					return false;

				// A buffer is required to be assigned at this point
				if (!errorState.check(binding->hasBuffer(), utility::stringFormat("No valid buffer was assigned to shader variable '%s' in base material '%s'", declaration.mName.c_str(), getMaterial()->mID.c_str()).c_str()))
					return false;
			}

			VkDescriptorBufferInfo& buffer_info = mStorageDescriptors[ssbo_index];
			buffer_info.buffer = binding->getBuffer().getBuffer();
			buffer_info.offset = 0;
			buffer_info.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet& ssbo_descriptor = mStorageWriteDescriptorSets.emplace_back();
			ssbo_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ssbo_descriptor.dstSet = VK_NULL_HANDLE;
			ssbo_descriptor.dstBinding = declaration.mBinding;
			ssbo_descriptor.dstArrayElement = 0;
			ssbo_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			ssbo_descriptor.descriptorCount = 1;
			ssbo_descriptor.pBufferInfo = mStorageDescriptors.data() + ssbo_index;

			++ssbo_index;
		}
 		return true;
	}


	bool BaseMaterialInstance::initSamplers(BaseMaterialInstanceResource& instanceResource, utility::ErrorState& errorState)
	{
		BaseMaterial* material = getMaterial();
		const SamplerDeclarations& sampler_declarations = material->getShader().getSamplerDeclarations();

		int num_sampler_images = 0;
		for (const SamplerDeclaration& declaration : sampler_declarations)
			num_sampler_images += declaration.mNumElements;

		mSamplerWriteDescriptorSets.resize(sampler_declarations.size());
		mSamplerDescriptors.reserve(num_sampler_images);	// We reserve to ensure that pointers remain consistent during the iteration

		// Samplers are initialized in two steps (somewhat similar to how uniforms are setup):
		// 1) We create sampler instances based on sampler declarations for all properties in MaterialInstance (so, the ones that are overridden).
		// 2) We initialize a VkWriteDescriptorSet that contains information that is either pointing to data from MaterialInstance if overridden, 
		//    otherwise data from the Material. This VkWriteDescriptorSet is used later to update the final DescriptorSet that will be used for 
		//    rendering. The VkWriteDescriptorSet is not yet complete because the final DescriptorSet is not yet set (it is nullptr). The reason is 
		//    that the DescriptorSet is acquired before rendering (see comments in update()).
		//
		// The VkWriteDescriptorSets require information about the images that are bound in the form of a VkDescriptorImageInfo. Each bound image 
		// needs such a structure. We build these here as well. Notice also that in the case of arrays, ImageInfo is created for each element in the array.
		// So imagine that we have a normal sampler, and a sampler array in the shader which has 10 elements. In this case we'd have:
		// - 2 sampler declarations in the shader, 2 sampler instances and 2 sampler descriptors in MaterialInstance
		// - 11 elements in the mSamplerImages array
		// The two sampler descriptors in the mSamplerDescriptors will contain count and offset info into the mSamplerImages array.
		// 
		// Notice that most of the information in VkWriteDescriptorSets remains constant: everything regarding the shader layout and the bindings do 
		// not change. So we build these write descriptor sets up front and only change the information that can change. These are:
		// - WriteDescriptorSet (which is acquired during update())
		// - What texture is bound (so: image info)
		//  
		for (int sampler_index = 0; sampler_index < sampler_declarations.size(); ++sampler_index)
		{
			const SamplerDeclaration& declaration = sampler_declarations[sampler_index];

			// Check if the sampler is set as override in the MaterialInstance
			const Sampler* sampler = findSamplerResource(instanceResource.mSamplers, declaration);
			SamplerInstance* sampler_instance = nullptr;
			if (sampler != nullptr)
			{
				// Sampler is overridden, make an SamplerInstance object
				std::unique_ptr<SamplerInstance> sampler_instance_override;
				if (declaration.mIsArray)
				{
                    switch (declaration.mType)
					{
					case SamplerDeclaration::EType::Type_2D:
					{
						sampler_instance_override = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, static_cast<const Sampler2DArray*>(sampler),
							std::bind(&MaterialInstance::onSamplerChanged, this, static_cast<int>(mSamplerDescriptors.size()), std::placeholders::_1, std::placeholders::_2));
						break;
					}
					case SamplerDeclaration::EType::Type_Cube:
					{
						sampler_instance_override = std::make_unique<SamplerCubeArrayInstance>(*mRenderService, declaration, static_cast<const SamplerCubeArray*>(sampler),
							std::bind(&MaterialInstance::onSamplerChanged, this, static_cast<int>(mSamplerDescriptors.size()), std::placeholders::_1, std::placeholders::_2));
						break;
					}
					default:
						errorState.fail("Unsupported sampler declaration type");
						return false;
					}
				}
				else
				{
					switch (declaration.mType)
					{
					case SamplerDeclaration::EType::Type_2D:
					{
						sampler_instance_override = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, static_cast<const Sampler2D*>(sampler),
							std::bind(&MaterialInstance::onSamplerChanged, this, static_cast<int>(mSamplerDescriptors.size()), std::placeholders::_1, std::placeholders::_2));
						break;
					}
					case SamplerDeclaration::EType::Type_Cube:
					{
						sampler_instance_override = std::make_unique<SamplerCubeInstance>(*mRenderService, declaration, static_cast<const SamplerCube*>(sampler),
							std::bind(&MaterialInstance::onSamplerChanged, this, static_cast<int>(mSamplerDescriptors.size()), std::placeholders::_1, std::placeholders::_2));
						break;
					}
					default:
						errorState.fail("Unsupported sampler declaration type");
						return false;
					}
				}

				if (!sampler_instance_override->init(errorState))
					return false;

				sampler_instance = sampler_instance_override.get();
				addSamplerInstance(std::move(sampler_instance_override));
			}
			else
			{
				// Sampler is not overridden, find it in the Material
				sampler_instance = material->findSampler(declaration.mName);
			}

			// Store the offset into the mSamplerImages array. This can either be the first index of an array, or just the element itself if it's not
			size_t sampler_descriptor_start_index = mSamplerDescriptors.size();
			VkSampler vk_sampler = sampler_instance->getVulkanSampler();
			if (declaration.mIsArray)
			{
				// Create all VkDescriptorImageInfo for all elements in the array
				switch(declaration.mType)
				{
				case SamplerDeclaration::EType::Type_2D:
				{
					Sampler2DArrayInstance* sampler_2d_array = static_cast<Sampler2DArrayInstance*>(sampler_instance);
					for (int index = 0; index < sampler_2d_array->getNumElements(); ++index)
					{
						const auto& tex = sampler_2d_array->hasTexture(index) ? sampler_2d_array->getTexture(index) : mRenderService->getEmptyTexture2D();
						addImageInfo(tex, vk_sampler);
					}
					break;
				}
				case SamplerDeclaration::EType::Type_Cube:
				{
					SamplerCubeArrayInstance* sampler_cube_array = static_cast<SamplerCubeArrayInstance*>(sampler_instance);
					for (int index = 0; index < sampler_cube_array->getNumElements(); ++index)
					{
						const auto& tex = sampler_cube_array->hasTexture(index) ? sampler_cube_array->getTexture(index) : mRenderService->getEmptyTextureCube();
						addImageInfo(tex, vk_sampler);
					}
					break;
				}
				default:
					errorState.fail("Unsupported sampler declaration type");
					return false;
				}
			}
			else
			{
				// Create a single VkDescriptorImageInfo for just this element
				switch (declaration.mType)
				{
				case SamplerDeclaration::EType::Type_2D:
				{
					Sampler2DInstance* sampler_2d = static_cast<Sampler2DInstance*>(sampler_instance);
					const auto& tex = sampler_2d->hasTexture() ? sampler_2d->getTexture() : mRenderService->getEmptyTexture2D();
					addImageInfo(tex, vk_sampler);
					break;
				}
				case SamplerDeclaration::EType::Type_Cube:
				{
					SamplerCubeInstance* sampler_cube = static_cast<SamplerCubeInstance*>(sampler_instance);
					const auto& tex = sampler_cube->hasTexture() ? sampler_cube->getTexture() : mRenderService->getEmptyTextureCube();
					addImageInfo(tex, vk_sampler);
					break;
				}
				default:
					errorState.fail("Unsupported sampler type");
					return false;
				}
			}

			// Create the write descriptor set. This set points to either a single element for non-arrays, or a list of contiguous elements for arrays.
			VkWriteDescriptorSet& write_descriptor_set = mSamplerWriteDescriptorSets[sampler_index];
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = VK_NULL_HANDLE;
			write_descriptor_set.dstBinding = sampler_instance->getDeclaration().mBinding;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.descriptorCount = mSamplerDescriptors.size() - sampler_descriptor_start_index;
			write_descriptor_set.pImageInfo = mSamplerDescriptors.data() + sampler_descriptor_start_index;
		}

		return true;
	}


	bool BaseMaterialInstance::initConstants(BaseMaterialInstanceResource& resource, utility::ErrorState& errorState)
	{
		BaseMaterial* material = getMaterial();
		const auto& declarations = material->getShader().getConstantDeclarations();

		for (const auto& declaration : declarations)
		{
			// Check if the constant is set as override in the MaterialInstanceResource
			const ShaderConstant* constant = findConstantResource(resource.mConstants, declaration);
			ShaderConstantInstance* constant_instance = nullptr;
			if (constant != nullptr)
			{
				// Shader Constant is overridden, create a new ShaderConstantInstance object
				auto constant_instance_override = std::make_unique<ShaderConstantInstance>(declaration, constant);
				constant_instance = &addConstantInstance(std::move(constant_instance_override));
			}
			else
			{
				constant_instance = material->findConstant(declaration.mName);
			}

			// If a constant is overriden
			if (constant_instance != nullptr)
			{		
				auto it = mShaderStageConstantMap.find(constant_instance->mDeclaration.mStage);
				if (it != mShaderStageConstantMap.end())
				{
					// Insert entry in the constant map associated with the specified stage
					it->second.insert({ constant_instance->mDeclaration.mConstantID, constant_instance->mValue });
				}
				else
				{
					// Create new map for the specified stage and insert entry
					ShaderConstantMap const_map = { { constant_instance->mDeclaration.mConstantID, constant_instance->mValue } };
					mShaderStageConstantMap.insert({ constant_instance->mDeclaration.mStage, std::move(const_map) });
				}
			}
		}

		// Recompute the shader constant hash used to create a pipeline key
		mConstantHash = 0;
		for (const auto& entry : mShaderStageConstantMap)
		{
			auto stage = entry.first;
			const auto& constant_map = entry.second;
			for (const auto& constant : constant_map)
			{
				const auto& value = constant.second;
				mConstantHash ^= std::hash<uint>{}(value);
			}
		}
		
		return true;
	}


	void BaseMaterialInstance::updateSamplers(const DescriptorSet& descriptorSet)
	{
		// We acquired 'some' compatible DescriptorSet with unknown contents. The dstSet must be overwritten
		// with the actual set that was acquired.
		// The actual latest images were already set correctly in mSamplerDescriptors during init and when setting
		// a new texture for a sampler. We just need to call VkUpdateDescriptors with the correct descriptorSet and
		// latest image info.
		for (VkWriteDescriptorSet& write_descriptor : mSamplerWriteDescriptorSets)
			write_descriptor.dstSet = descriptorSet.mSet;

		vkUpdateDescriptorSets(mDevice, mSamplerWriteDescriptorSets.size(), mSamplerWriteDescriptorSets.data(), 0, nullptr);
	}


	void BaseMaterialInstance::updateBuffers(const DescriptorSet& descriptorSet)
	{
		// We acquired 'some' compatible DescriptorSet with unknown contents. The dstSet must be overwritten
		// with the actual set that was acquired.
		// The actual latest buffers were already set correctly in mStorageDescriptors during init and when setting
		// a new buffer for a binding. We just need to call VkUpdateDescriptors with the correct descriptorSet and
		// latest buffer info.
		for (VkWriteDescriptorSet& write_descriptor : mStorageWriteDescriptorSets)
			write_descriptor.dstSet = descriptorSet.mSet;

		vkUpdateDescriptorSets(mDevice, mStorageWriteDescriptorSets.size(), mStorageWriteDescriptorSets.data(), 0, nullptr);
	}


	bool BaseMaterialInstance::initInternal(RenderService& renderService, BaseMaterial& material, BaseMaterialInstanceResource& instanceResource, utility::ErrorState& errorState)
	{
		mDevice = renderService.getDevice();
		mRenderService = &renderService;
		mMaterial = &material;
		mResource = &instanceResource;

		const auto& shader = mMaterial->getShader();

		// Here we create UBOs in two parts:
		// 1) We create a hierarchical uniform instance structure based on the hierarchical declaration structure from the shader. We do
		//    this only for the properties in the MaterialInstance (so: the properties that were overridden). We've also already done this
		//    in the Material, so after this pass we have a hierarchical structure in Material for all properties, and a similar structure
		//    for the MaterialInstance, but only for the properties that we've overridden.
		// 2) After pass 1, we create the UBO, which is a non-hierarchical structure that holds pointers to all leaf elements. These leaf
		//    elements can point to either Material or MaterialInstance instance uniforms, depending on whether the property was overridden.
		//    Notice that this also means that this structure should be rebuilt when a 'new' override is made at runtime. This is handled in
		//    update() by rebuilding the UBO when a new uniform is created.
		const std::vector<BufferObjectDeclaration>& ubo_declarations = shader.getUBODeclarations();
		for (const BufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			const UniformStruct* struct_resource = rtti_cast<const UniformStruct>(findUniformStructMember(instanceResource.mUniforms, ubo_declaration));

			// Pass 1: create hierarchical structure
			UniformStructInstance* override_struct = nullptr;
			if (struct_resource != nullptr)
			{
				override_struct = &createUniformRootStruct(ubo_declaration, std::bind(&BaseMaterialInstance::onUniformCreated, this));
				if (!override_struct->addUniformRecursive(ubo_declaration, struct_resource, std::bind(&BaseMaterialInstance::onUniformCreated, this), false, errorState))
					return false;
			}

			// Verify buffer object type
			if (!errorState.check(ubo_declaration.mDescriptorType == EDescriptorType::Uniform, utility::stringFormat("Buffer Object Type mismatch in shader declaration %s", ubo_declaration.mName.c_str())))
				return false;

			// Pass 2: gather leaf uniform instances for a single ubo
			UniformBufferObject ubo(ubo_declaration);
			rebuildUBO(ubo, override_struct);

			mUniformBufferObjects.emplace_back(std::move(ubo));
		}
		mUniformsCreated = false;

		if (!initBuffers(instanceResource, errorState))
			return false;

		if (!initSamplers(instanceResource, errorState))
			return false;

		if (!initConstants(instanceResource, errorState))
			return false;

		// We get/create an allocator that is compatible with the layout of the shader that this material is bound to. Practically this
		// means a descriptor with:
		// - Same number of UBOs and samplers
		// - Same layout bindings
		// So, any MaterialInstance that is bound to the same shader will be able to allocate from the same DescriptorSetAllocator. It is even
		// possible that multiple shaders that have the same bindings, number of UBOs and samplers can share the same allocator. This is advantageous
		// because internally, pools are created that are allocated from. We want as little empty space in those pools as possible (we want the allocators
		// to act as 'globally' as possible).
		mDescriptorSetCache = &renderService.getOrCreateDescriptorSetCache(shader.getDescriptorSetLayout());

		return true;
	}


	const DescriptorSet& BaseMaterialInstance::update()
	{
		// The UBO contains pointers to all leaf uniform instances. These can be either defines in the material or the 
		// material instance, depending on whether it's overridden. If new overrides were created between update calls,
		// we need to patch pointers in the UBO structure to make sure they point to the correct instances.
		if (mUniformsCreated)
		{
			for (UniformBufferObject& ubo : mUniformBufferObjects)
				rebuildUBO(ubo, findUniform(ubo.mDeclaration->mName));

			mUniformsCreated = false;
		}

		// The DescriptorSet contains information about all UBOs and samplers, along with the buffers that are bound to it.
		// We acquire a descriptor set that is compatible with our shader. The allocator holds a number of allocated descriptor
		// sets and we acquire one that is not in use anymore (that is not in any active command buffer). We cannot make assumptions
		// about the contents of the descriptor sets. The UBO buffers that are bound to it may have different contents than our
		// MaterialInstance, and the samplers may be bound to different images than those that are currently set in the MaterialInstance.
		// For this reason, we always fully update uniforms and samplers to make the descriptor set up to date with the MaterialInstance
		// contents.
		// The reason why we cannot make any assumptions about the contents of DescriptorSets in the cache is that we can perform multiple
		// updates & draws of a MaterialInstance within a single frame. How many draws we do for a MaterialInstance is unknown, that is 
		// up to the client. Because the MaterialInstance state changes *during* a frame for an unknown amount of draws, we 
		// cannot associate DescriptorSet state as returned from the allocator with the latest MaterialInstance state. One way of looking
		// at it is that MaterialInstance's state is 'volatile'. This means we cannot perform dirty checking.
		// One way to tackle this is by maintaining a hash for the uniform/sampler constants that is maintained both in the allocator for
		// a descriptor set and in MaterialInstance. We could then prefer to acquire descriptor sets that have matching hashes.
		const DescriptorSet& descriptor_set = mDescriptorSetCache->acquire(mUniformBufferObjects, mStorageDescriptors.size(), mSamplerDescriptors.size());

		updateUniforms(descriptor_set, mUniformBufferObjects);
		updateBuffers(descriptor_set);
		updateSamplers(descriptor_set);

		return descriptor_set;
	}


	//////////////////////////////////////////////////////////////////////////
	// MaterialInstance
	//////////////////////////////////////////////////////////////////////////

	bool MaterialInstance::init(RenderService& renderService, MaterialInstanceResource& resource, utility::ErrorState& errorState)
	{
		mResource = &resource;
		if (!initInternal(renderService, *resource.mMaterial, resource, errorState))
			return false;
		return true;
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
	// ComputeMaterialInstance
	//////////////////////////////////////////////////////////////////////////

	bool ComputeMaterialInstance::init(RenderService& renderService, ComputeMaterialInstanceResource& resource, utility::ErrorState& errorState)
	{
		mResource = &resource;
		if (!initInternal(renderService,*resource.mComputeMaterial, resource, errorState))
			return false;

		return true;
	}


	glm::uvec3 ComputeMaterialInstance::getWorkGroupSize() const
	{
		// Fetch work group size overrides
		const auto& override_map = getMaterial().getShader().getWorkGroupSizeOverrides();
		glm::uvec3 workgroup_size = getMaterial().getWorkGroupSize();
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
