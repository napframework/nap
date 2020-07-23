// Local Includes
#include "materialinstance.h"
#include "mesh.h"
#include "material.h"

// External includes
#include <nap/logger.h>
#include "rtti/rttiutilities.h"
#include "renderservice.h"
#include "uniforminstance.h"
#include "descriptorsetcache.h"
#include "vk_mem_alloc.h"


RTTI_BEGIN_CLASS(nap::MaterialInstanceResource)
	RTTI_PROPERTY("Material",					&nap::MaterialInstanceResource::mMaterial,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Uniforms",					&nap::MaterialInstanceResource::mUniforms,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Samplers",					&nap::MaterialInstanceResource::mSamplers,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("BlendMode",					&nap::MaterialInstanceResource::mBlendMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",					&nap::MaterialInstanceResource::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::MaterialInstance)
	RTTI_FUNCTION("getOrCreateUniform",			(nap::UniformStructInstance* (nap::MaterialInstance::*)(const std::string&)) &nap::MaterialInstance::getOrCreateUniform);
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
	const Sampler* findSamplerResource(const std::vector<T>& samplers, const SamplerDeclaration& declaration)
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


	//////////////////////////////////////////////////////////////////////////
	// MaterialInstance
	//////////////////////////////////////////////////////////////////////////

	UniformStructInstance* MaterialInstance::getOrCreateUniform(const std::string& name)
	{
		UniformStructInstance* existing = findUniform(name);
		if (existing != nullptr)
			return existing;

		// Find the declaration in the shader (if we can't find it, it's not a name that actually exists in the shader, which is an error).
		const UniformStructDeclaration* declaration = nullptr;
		const std::vector<UniformBufferObjectDeclaration>& ubo_declarations = getMaterial().getShader().getUBODeclarations();
		for (const UniformBufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			if (ubo_declaration.mName == name)
			{
				declaration = &ubo_declaration;
				break;
			}
		}

		if (declaration == nullptr)
			return nullptr;

		// At the MaterialInstance level, we always have UBOs at the root, so we create a root struct
		return &createRootStruct(*declaration, std::bind(&MaterialInstance::onUniformCreated, this));
	}


	void MaterialInstance::onUniformCreated()
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
	void MaterialInstance::onSamplerChanged(int imageStartIndex, SamplerInstance& samplerInstance)
	{
		VkSampler vk_sampler = samplerInstance.getVulkanSampler();
		if (samplerInstance.get_type() == RTTI_OF(Sampler2DArrayInstance))
		{
			Sampler2DArrayInstance* sampler_2d_array = (Sampler2DArrayInstance*)(&samplerInstance);

			for (int index = 0; index < sampler_2d_array->getNumElements(); ++index)
			{
				const Texture2D& texture = sampler_2d_array->getTexture(index);

				VkDescriptorImageInfo& imageInfo = mSamplerWriteDescriptors[imageStartIndex + index];
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = texture.getImageView();
				imageInfo.sampler = vk_sampler;
			}
		}
		else
		{
			Sampler2DInstance* sampler_2d = (Sampler2DInstance*)(&samplerInstance);

			VkDescriptorImageInfo& imageInfo = mSamplerWriteDescriptors[imageStartIndex];
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = sampler_2d->getTexture().getImageView();
			imageInfo.sampler = vk_sampler;
		}
	}


	void MaterialInstance::rebuildUBO(UniformBufferObject& ubo, UniformStructInstance* overrideStruct)
	{
		ubo.mUniforms.clear();

		const UniformStructInstance* base_struct = rtti_cast<const UniformStructInstance>(getMaterial().findUniform(ubo.mDeclaration->mName));
		assert(base_struct != nullptr);

		buildUniformBufferObjectRecursive(*base_struct, overrideStruct, ubo);
	}


	void MaterialInstance::updateUniforms(const DescriptorSet& descriptorSet)
	{
		// Go over all the UBOs and memcpy the latest MaterialInstance state into the allocated descriptorSet's VkBuffers
		for (int ubo_index = 0; ubo_index != descriptorSet.mBuffers.size(); ++ubo_index)
		{
			UniformBufferObject& ubo = mUniformBufferObjects[ubo_index];
			VmaAllocationInfo allocation = descriptorSet.mBuffers[ubo_index].mAllocationInfo;
			
			void* mapped_memory = allocation.pMappedData;

			for (auto& uniform : ubo.mUniforms)
				uniform->push((uint8_t*)mapped_memory);
		}
	}


	void MaterialInstance::addImageInfo(const Texture2D& texture2D, VkSampler sampler)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture2D.getImageView();
		imageInfo.sampler = sampler;

		mSamplerWriteDescriptors.push_back(imageInfo);
	}

	bool MaterialInstance::initSamplers(utility::ErrorState& errorState)
	{
		Material& material = *mResource->mMaterial;
		const Shader& shader = material.getShader();
		const SamplerDeclarations& sampler_declarations = shader.getSamplerDeclarations();

		int num_sampler_images = 0;
		for (const SamplerDeclaration& declaration : sampler_declarations)
			num_sampler_images += declaration.mNumArrayElements;

		mSamplerWriteDescriptorSets.resize(sampler_declarations.size());
		mSamplerWriteDescriptors.reserve(num_sampler_images);	// We reserve to ensure that pointers remain consistent during the iteration
		
		Texture2D& emptyTexture = mRenderService->getEmptyTexture();

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
			bool is_array = declaration.mNumArrayElements > 1;

			// Check if the sampler is set as override in the MaterialInstance
			const Sampler* sampler = findSamplerResource(mResource->mSamplers, declaration);
			SamplerInstance* sampler_instance = nullptr;
			if (sampler != nullptr)
			{
				// Sampler is overridden, make an SamplerInstance object
				std::unique_ptr<SamplerInstance> sampler_instance_override;
				if (is_array)
					sampler_instance_override = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, (Sampler2DArray*)sampler, std::bind(&MaterialInstance::onSamplerChanged, this, (int)mSamplerWriteDescriptors.size(), std::placeholders::_1));
				else
					sampler_instance_override = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, (Sampler2D*)sampler, std::bind(&MaterialInstance::onSamplerChanged, this, (int)mSamplerWriteDescriptors.size(), std::placeholders::_1));

				if (!sampler_instance_override->init(errorState))
					return false;

				sampler_instance = sampler_instance_override.get();
				addSamplerInstance(std::move(sampler_instance_override));
			}
			else
			{
				// Sampler is not overridden, find it in the Material
				sampler_instance = material.findSampler(declaration.mName);
			}

			// Store the offset into the mSamplerImages array. This can either be the first index of an array, or just the element itself if it's not
			size_t sampler_descriptor_start_index = mSamplerWriteDescriptors.size();
			VkSampler vk_sampler = sampler_instance->getVulkanSampler();
			if (is_array)
			{
				// Create all VkDescriptorImageInfo for all elements in the array
				Sampler2DArrayInstance* sampler_2d_array = (Sampler2DArrayInstance*)(sampler_instance);

				for (int index = 0; index < sampler_2d_array->getNumElements(); ++index)
				{
					if (sampler_2d_array->hasTexture(index))
						addImageInfo(sampler_2d_array->getTexture(index), vk_sampler);
					else
						addImageInfo(emptyTexture, vk_sampler);
				}
			}
			else
			{
				// Create a single VkDescriptorImageInfo for just this element
				Sampler2DInstance* sampler_2d = (Sampler2DInstance*)(sampler_instance);
				
				if (sampler_2d->hasTexture())
					addImageInfo(sampler_2d->getTexture(), vk_sampler);
				else
					addImageInfo(emptyTexture, vk_sampler);
			}

			// Create the write descriptor set. This set points to either a single element for non-arrays, or a list of contiguous elements for arrays.
			VkWriteDescriptorSet& write_descriptor_set = mSamplerWriteDescriptorSets[sampler_index];
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = nullptr;
			write_descriptor_set.dstBinding = sampler_instance->getDeclaration().mBinding;
			write_descriptor_set.dstArrayElement = 0;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.descriptorCount = mSamplerWriteDescriptors.size() - sampler_descriptor_start_index;
			write_descriptor_set.pImageInfo = mSamplerWriteDescriptors.data() + sampler_descriptor_start_index;
		}

		return true;
	}


	SamplerInstance* MaterialInstance::getOrCreateSamplerInternal(const std::string& name)
	{
		// See if we have an override in MaterialInstance. If so, we can return it
		SamplerInstance* existing_sampler = findSampler(name);
		if (existing_sampler != nullptr)
			return existing_sampler;

		SamplerInstance* result = nullptr;

		const SamplerDeclarations& sampler_declarations = getMaterial().getShader().getSamplerDeclarations();
		int image_start_index = 0;
		for (const SamplerDeclaration& declaration : sampler_declarations)
		{
			if (declaration.mName == name)
			{
				bool is_array = declaration.mNumArrayElements > 1;

				std::unique_ptr<SamplerInstance> sampler_instance_override;
				if (is_array)
					sampler_instance_override = std::make_unique<Sampler2DArrayInstance>(*mRenderService, declaration, nullptr, std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1));
				else
					sampler_instance_override = std::make_unique<Sampler2DInstance>(*mRenderService, declaration, nullptr, std::bind(&MaterialInstance::onSamplerChanged, this, image_start_index, std::placeholders::_1));

				utility::ErrorState error_state;
				bool initialized = sampler_instance_override->init(error_state);
				assert(initialized);

				result = sampler_instance_override.get();

				addSamplerInstance(std::move(sampler_instance_override));
				break;
			}

			image_start_index += declaration.mNumArrayElements;
		}
		return result;
	}


	void MaterialInstance::updateSamplers(const DescriptorSet& descriptorSet)
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


	VkDescriptorSet MaterialInstance::update()
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
		const DescriptorSet& descriptor_set = mDescriptorSetCache->acquire(mUniformBufferObjects, mSamplerWriteDescriptors.size());

		updateUniforms(descriptor_set);
		updateSamplers(descriptor_set);

		return descriptor_set.mSet;
	}


	bool MaterialInstance::init(RenderService& renderService, MaterialInstanceResource& resource, utility::ErrorState& errorState)
	{
		mResource = &resource;
		mDevice = renderService.getDevice();
		mRenderService = &renderService;

		Material& material = *resource.mMaterial;
		const Shader& shader = material.getShader();

		// Here we create UBOs in two parts:
		// 1) We create a hierarchical uniform instance structure based on the hierarchical declaration structure from the shader. We do
		//    this only for the properties in the MaterialInstance (so: the properties that were overridden). We've also already done this
		//    in the Material, so after this pass we have a hierarchical structure in Material for all properties, and a similar structure
		//    for the MaterialInstance, but only for the properties that we've overridden.
		// 2) After pass 1, we create the UBO, which is a non-hierarchical structure that holds pointers to all leaf elements. These leaf
		//    elements can point to either Material or MaterialInstance instance uniforms, depending on whether the property was overridden.
		//    Notice that this also means that this structure should be rebuild when a 'new' override is made at runtime. This is handled in
		//    update() by rebuilding the UBO when a new uniform is created.
		const std::vector<UniformBufferObjectDeclaration>& ubo_declarations = shader.getUBODeclarations();
		for (const UniformBufferObjectDeclaration& ubo_declaration : ubo_declarations)
		{
			const UniformStruct* struct_resource = rtti_cast<const UniformStruct>(findUniformStructMember(resource.mUniforms, ubo_declaration));

			// Pass 1: create hierarchical structure
			UniformStructInstance* override_struct = nullptr;
			if (struct_resource != nullptr)
			{
				override_struct = &createRootStruct(ubo_declaration, std::bind(&MaterialInstance::onUniformCreated, this));
				if (!override_struct->addUniformRecursive(ubo_declaration, struct_resource, std::bind(&MaterialInstance::onUniformCreated, this), false, errorState))
					return false;
			}

			// Pass 2: gather leaf uniform instances for a single ubo
			UniformBufferObject ubo(ubo_declaration);
			rebuildUBO(ubo, override_struct);
			mUniformBufferObjects.emplace_back(std::move(ubo));
		}
		
		mUniformsCreated = false;
				
		if (!initSamplers(errorState))
			return false;

		// We get/create an allocator that is compatible with the layout of the shader that this material is bound to. Practically this
		// means a descriptor with:
		// - Same number of UBOs and samplers
		// - Same layout bindings
		// So, any MaterialInstance that is bound to the same shader will be able to allocate from the same DescriptorSetAllocator. It is even
		// possible that multiple shaders that have the same bindings, number of UBOs and samplers can share the same allocator. This is advantageous
		// because internally, pools are created that are allocated from. We want as little empty space in those pools as possible (we want the allocators
		// to act as 'globally' as possible).
		mDescriptorSetCache = &mRenderService->getOrCreateDescriptorSetCache(getMaterial().getShader().getDescriptorSetLayout());

		return true;
	}


	Material& MaterialInstance::getMaterial() 
	{ 
		return *mResource->mMaterial; 
	}


	const nap::Material& MaterialInstance::getMaterial() const
	{
		return *mResource->mMaterial;
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
}