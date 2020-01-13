// Local Includes
#include "materialinstance.h"
#include "nshader.h"
#include "mesh.h"

// External includes
#include <nap/logger.h>
#include <GL/glew.h>
#include "rtti/rttiutilities.h"
#include "renderservice.h"
#include "nshaderutils.h"
#include "uniforminstances.h"
#include "vk_mem_alloc.h"


RTTI_BEGIN_CLASS(nap::MaterialInstanceResource)
	RTTI_PROPERTY("Material",					&nap::MaterialInstanceResource::mMaterial,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Uniforms",					&nap::MaterialInstanceResource::mUniforms,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Samplers",					&nap::MaterialInstanceResource::mSamplers,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("BlendMode",					&nap::MaterialInstanceResource::mBlendMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",					&nap::MaterialInstanceResource::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MaterialInstance)
	RTTI_FUNCTION("getOrCreateUniform",			(nap::Uniform* (nap::MaterialInstance::*)(const std::string&)) &nap::MaterialInstance::getOrCreateUniform);
RTTI_END_CLASS

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
	const Sampler* findSamplerResource(const std::vector<T>& samplers, const opengl::SamplerDeclaration& declaration)
	{
		for (auto& sampler : samplers)
			if (sampler->mName == declaration.mName)
				return sampler.get();

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
			else if (declaration_type == RTTI_OF(UniformValueArrayInstance))
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


	//////////////////////////////////////////////////////////////////////////
	// MaterialInstance
	//////////////////////////////////////////////////////////////////////////

	UniformStructInstance& MaterialInstance::getOrCreateUniform(const std::string& name)
	{
		UniformStructInstance* existing = findUniform(name);
		if (existing != nullptr)
			return *existing;

		const opengl::UniformStructDeclaration* declaration = nullptr;
		const std::vector<opengl::UniformBufferObjectDeclaration>& ubo_declarations = getMaterial()->getShader()->getShader().getUBODeclarations();
		for (const opengl::UniformBufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			if (ubo_declaration.mName == name)
			{
				declaration = &ubo_declaration;
				break;
			}
		}
		assert(declaration != nullptr);

		return createRootStruct(*declaration, std::bind(&MaterialInstance::onUniformCreated, this));
	}


	void MaterialInstance::onUniformCreated()
	{
		mUniformsDirty = true;
	}


	void MaterialInstance::onSamplerChanged(int imageStartIndex, SamplerInstance& samplerInstance)
	{
		size_t sampler_image_start_index = mSamplerImages.size();
		if (samplerInstance.get_type() == RTTI_OF(Sampler2DArrayInstance))
		{
			Sampler2DArrayInstance* sampler_2d_array = (Sampler2DArrayInstance*)(&samplerInstance);

			for (int index = imageStartIndex; index < sampler_2d_array->getNumElements(); ++index)
			{
				const Texture2D& texture = sampler_2d_array->getTexture(index);

				VkDescriptorImageInfo& imageInfo = mSamplerImages[index];
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = texture.getImageView();
				imageInfo.sampler = sampler_2d_array->getSampler();
			}
		}
		else
		{
			Sampler2DInstance* sampler_2d = (Sampler2DInstance*)(&samplerInstance);

			VkDescriptorImageInfo& imageInfo = mSamplerImages[imageStartIndex];
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = sampler_2d->getTexture().getImageView();
			imageInfo.sampler = sampler_2d->getSampler();
		}
	}


	void MaterialInstance::rebuildUBO(UniformBufferObject& ubo, UniformStructInstance* overrideStruct)
	{
		ubo.mUniforms.clear();

		const UniformStructInstance* base_struct = rtti_cast<const UniformStructInstance>(getMaterial()->findUniform(ubo.mDeclaration->mName));
		assert(base_struct != nullptr);

		buildUniformBufferObjectRecursive(*base_struct, overrideStruct, ubo);
	}


	void MaterialInstance::updateUniforms(const DescriptorSet& descriptorSet)
	{
		VkDevice device = getMaterial()->getRenderer().getDevice();

		VmaAllocator allocator = mRenderService->getVulkanAllocator();

		for (int ubo_index = 0; ubo_index != descriptorSet.mBuffers.size(); ++ubo_index)
		{
			UniformBufferObject& ubo = mUniformBufferObjects[ubo_index];
			VmaAllocationInfo allocation = descriptorSet.mBuffers[ubo_index].mAllocationInfo;
			
			void* mapped_memory = allocation.pMappedData;

			for (auto& uniform : ubo.mUniforms)
				uniform->push((uint8_t*)mapped_memory);
		}
	}


	bool MaterialInstance::initSamplers(utility::ErrorState& errorState)
	{
		Material& material = *mResource->mMaterial;
		const opengl::Shader& shader = material.getShader()->getShader();

		const opengl::SamplerDeclarations& sampler_declarations = shader.getSamplerDeclarations();

		int num_sampler_images = 0;
		for (const opengl::SamplerDeclaration& declaration : sampler_declarations)
			num_sampler_images += declaration.mNumArrayElements;

		mSamplerDescriptors.resize(sampler_declarations.size());
		mSamplerImages.reserve(num_sampler_images);	// We reserve to ensure that pointers remain consistent during the iteration
		
		for (int sampler_index = 0; sampler_index < sampler_declarations.size(); ++sampler_index)
		{
			const opengl::SamplerDeclaration& declaration = sampler_declarations[sampler_index];
			bool is_array = declaration.mNumArrayElements > 1;

			const Sampler* sampler = findSamplerResource(mResource->mSamplers, declaration);
			SamplerInstance* sampler_instance = nullptr;
			if (sampler != nullptr)
			{
				std::unique_ptr<SamplerInstance> sampler_instance_override;
				if (is_array)
					sampler_instance_override = std::make_unique<Sampler2DArrayInstance>(mDevice, declaration, (Sampler2DArray*)sampler, std::bind(&MaterialInstance::onSamplerChanged, this, (int)mSamplerImages.size(), std::placeholders::_1));
				else
					sampler_instance_override = std::make_unique<Sampler2DInstance>(mDevice, declaration, (Sampler2D*)sampler, std::bind(&MaterialInstance::onSamplerChanged, this, (int)mSamplerImages.size(), std::placeholders::_1));

				if (!sampler_instance_override->init(errorState))
					return false;

				sampler_instance = sampler_instance_override.get();
				addSamplerInstance(std::move(sampler_instance_override));
			}
			else
			{
				sampler_instance = material.findSampler(declaration.mName);
			}

			size_t sampler_image_start_index = mSamplerImages.size();
			if (is_array)
			{
				Sampler2DArrayInstance* sampler_2d_array = (Sampler2DArrayInstance*)(sampler_instance);

				for (int index = 0; index < sampler_2d_array->getNumElements(); ++index)
				{
					const Texture2D& texture = sampler_2d_array->getTexture(index);

					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = texture.getImageView();
					imageInfo.sampler = sampler_instance->getSampler();

					mSamplerImages.push_back(imageInfo);
				}
			}
			else
			{
				Sampler2DInstance* sampler_2d = (Sampler2DInstance*)(sampler_instance);

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = sampler_2d->getTexture().getImageView();
				imageInfo.sampler = sampler_instance->getSampler();

				mSamplerImages.push_back(imageInfo);
			}

			VkWriteDescriptorSet& write_descriptor_set = mSamplerDescriptors[sampler_index];
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = nullptr;
			write_descriptor_set.dstBinding = sampler_instance->getDeclaration().mBinding;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.descriptorCount = mSamplerImages.size() - sampler_image_start_index;
			write_descriptor_set.pImageInfo = mSamplerImages.data() + sampler_image_start_index;

			mSamplers.push_back(sampler_instance);
		}

		return true;
	}


	SamplerInstance& MaterialInstance::getOrCreateSamplerInternal(const std::string& name)
	{
		SamplerInstance* existing_sampler = findSampler(name);
		if (existing_sampler != nullptr)
			return *existing_sampler;

		SamplerInstance* result = nullptr;

		int image_start_index = 0;
		for (int index = 0; index < mSamplers.size(); ++index)
		{
			const opengl::SamplerDeclaration& declaration = mSamplers[index]->getDeclaration();
			if (declaration.mName == name)
			{
				bool is_array = declaration.mNumArrayElements > 1;

				std::unique_ptr<SamplerInstance> sampler_instance_override;
				if (is_array)
					sampler_instance_override = std::make_unique<Sampler2DArrayInstance>(mDevice, declaration, nullptr, std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1));
				else
					sampler_instance_override = std::make_unique<Sampler2DInstance>(mDevice, declaration, nullptr, std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1));

				utility::ErrorState error_state;
				bool initialized = sampler_instance_override->init(error_state);
				assert(initialized);

				result = sampler_instance_override.get();
				mSamplers[index] = result;

				addSamplerInstance(std::move(sampler_instance_override));
				break;
			}

			image_start_index += declaration.mNumArrayElements;
		}

		assert(result != nullptr);
		return *result;
	}


	void MaterialInstance::updateSamplers(const DescriptorSet& descriptorSet)
	{
		for (VkWriteDescriptorSet& write_descriptor : mSamplerDescriptors)
			write_descriptor.dstSet = descriptorSet.mSet;

		vkUpdateDescriptorSets(mDevice, mSamplerDescriptors.size(), mSamplerDescriptors.data(), 0, nullptr);
	}


	VkDescriptorSet MaterialInstance::update()
	{
		if (mUniformsDirty)
		{
			for (UniformBufferObject& ubo : mUniformBufferObjects)
				rebuildUBO(ubo, findUniform(ubo.mDeclaration->mName));

			mUniformsDirty = false;
		}

		const DescriptorSet& descriptor_set = mDescriptorSetAllocator->acquire(mUniformBufferObjects, mSamplers);

		updateUniforms(descriptor_set);
		updateSamplers(descriptor_set);

		return descriptor_set.mSet;
	}


	bool MaterialInstance::init(RenderService& renderService, MaterialInstanceResource& resource, utility::ErrorState& errorState)
	{
		mResource = &resource;
		mDevice = renderService.getRenderer().getDevice();
		mRenderService = &renderService;

		Material& material = *resource.mMaterial;
		const opengl::Shader& shader = material.getShader()->getShader();

		const std::vector<opengl::UniformBufferObjectDeclaration>& ubo_declarations = shader.getUBODeclarations();
		for (const opengl::UniformBufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			const UniformStruct* struct_resource = rtti_cast<const UniformStruct>(findUniformStructMember(resource.mUniforms, ubo_declaration));

			UniformStructInstance* override_struct = nullptr;
			if (struct_resource != nullptr)
			{
				override_struct = &createRootStruct(ubo_declaration, std::bind(&MaterialInstance::onUniformCreated, this));
				if (!override_struct->addUniformRecursive(ubo_declaration, struct_resource, std::bind(&MaterialInstance::onUniformCreated, this), false, errorState))
					return false;
			}

			UniformBufferObject ubo(ubo_declaration);
			rebuildUBO(ubo, override_struct);
			mUniformBufferObjects.emplace_back(std::move(ubo));
		}
		
		mUniformsDirty = false;
				
		if (!initSamplers(errorState))
			return false;

		mDescriptorSetAllocator = &mRenderService->getOrCreateDescriptorSetAllocator(getMaterial()->getShader()->getShader().getDescriptorSetLayout());

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
		pipelineStateChanged(*this, *mRenderService);
	}

	
	void MaterialInstance::setDepthMode(EDepthMode depthMode)
	{
		mResource->mDepthMode = depthMode;
		pipelineStateChanged(*this, *mRenderService);
	}


	EDepthMode MaterialInstance::getDepthMode() const
	{
		if (mResource->mDepthMode != EDepthMode::NotSet)
			return mResource->mDepthMode;

		return mResource->mMaterial->getDepthMode();
	}
}