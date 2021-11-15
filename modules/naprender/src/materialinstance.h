/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resourceptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>
#include <nap/signalslot.h>

// Local includes
#include "uniformcontainer.h"
#include "descriptorsetcache.h"
#include "material.h"

namespace nap
{
	class Material;
	class Renderer;
	struct DescriptorSet;
	class DescriptorSetCache;

	/**
	 * MaterialInstanceResource is the 'resource' or 'data' counterpart of MaterialInstance, intended to be used 
	 * as fields in ComponentResources. The object needs to be passed to MaterialInstance's init() function.
	 */
	class NAPAPI BaseMaterialInstanceResource
	{
		RTTI_ENABLE()
	public:
		std::vector<ResourcePtr<UniformStruct>>			mUniforms;									///< Property: "Uniforms" uniform structs that you're overriding
		std::vector<ResourcePtr<StorageUniformStruct>>	mStorageUniforms;							///< Property: "StorageUniforms" storage uniform structs that you're overriding
		std::vector<ResourcePtr<Sampler>>				mSamplers;									///< Property: "Samplers" samplers that you're overriding
	};

	class NAPAPI MaterialInstanceResource : public BaseMaterialInstanceResource
	{
		RTTI_ENABLE(BaseMaterialInstanceResource)
	public:
		ResourcePtr<Material>						mMaterial;										///< Property: "Material" source material
		EBlendMode									mBlendMode = EBlendMode::NotSet;				///< Property: "BlendMode" Blend mode override. Uses source material blend mode by default
		EDepthMode									mDepthMode = EDepthMode::NotSet;				///< Property: "DepthMode" Depth mode override. Uses source material depth mode by default
	};

	class NAPAPI ComputeMaterialInstanceResource : public BaseMaterialInstanceResource
	{
		RTTI_ENABLE(BaseMaterialInstanceResource)
	public:
		ResourcePtr<ComputeMaterial>				mComputeMaterial;								///< Property: "ComputeMaterial" source material
	};

	/**
	 * BaseMaterialInstance
	 */
	class NAPAPI BaseMaterialInstance : public UniformContainer
	{
		RTTI_ENABLE(UniformContainer)
	public:
		/**
		 * Gets or creates a uniform struct (ubo) for this material instance.
		 * This means that the uniform returned is only applicable to this instance.
		 * In order to change a uniform so that it's value is shared among MaterialInstances, use getMaterial().getUniform().
		 *
		 * @param name: the name of the uniform struct (ubo) as declared in the shader.
		 * @return uniform that was found or created, nullptr if not available.
		 */
		virtual UniformStructInstance* getOrCreateUniform(const std::string& name);

		/**
		 * Gets or creates a shader storage uniform struct (ssbo) for this material instance.
		 * This means that the uniform returned is only applicable to this instance.
		 * In order to change a uniform so that it's value is shared among MaterialInstances, use getMaterial().getUniform().
		 *
		 * @param name: the name of the sorage uniform struct (ssbo) as declared in the shader.
		 * @return uniform that was found or created, nullptr if not available.
		 */
		virtual StorageUniformStructInstance* getOrCreateStorageUniform(const std::string& name);

		/**
		 * Gets or creates a nap::SamplerInstance of type T for this material instance.
		 * This means that the sampler returned is only applicable to this instance.
		 * In order to change a sampler so that it's value is shared among MaterialInstances, use getMaterial().findSampler().
		 * This function will assert if the name of the uniform does not match the type that you are trying to create.
		 *
		 * ~~~~~{.cpp}
		 * material_instance->getOrCreateSampler<nap::Sampler2DInstance>("inTexture");
		 * ~~~~~
		 *
		 * @param name: the name of the sampler declared in the shader.
		 * @return nap::SamplerInstance of type T, nullptr if not available.
		 */
		template<class T>
		T* getOrCreateSampler(const std::string& name);

		/**
		 * Gets or creates a nap::SamplerInstance for this material instance.
		 * This means that the sampler returned is only applicable to this instance.
		 * In order to change a sampler so that it's value is shared among MaterialInstances, use getMaterial().findSampler().
		 * This function will assert if the name of the uniform does not match the type that you are trying to create.
		 *
		 * @param name: the name of the sampler declared in the shader.
		 * @return nap::SamplerInstance of type T, nullptr if not available.
		 */
		SamplerInstance* getOrCreateSampler(const std::string& name) { return getOrCreateSamplerInternal(name); }

		/**
		 * @return base material that this instance is overriding
		 */
		virtual BaseMaterial* getBaseMaterial() = 0;

		/**
		 * @return base material that this instance is overriding
		 */
		virtual const BaseMaterial* getBaseMaterial() const = 0;

		/**
		 * @return base material instance resource
		 */
		virtual const BaseMaterialInstanceResource* getResource() const = 0;

		/**
		 * TODO: Currently keeping for backwards compatibility. The new update(std::vector<VkDescriptorSet>& outDescriptorSets) is recommended.
		 * This needs to be called before each draw. It will push the current uniform and sampler data into memory
		 * that is accessible for the GPU. A descriptor set will be returned that must be used in VkCmdBindDescriptorSets
		 * before the Vulkan draw call is issued.
		 *
		 * ~~~~~{.cpp}
		 *	VkDescriptorSet descriptor_set = mat_instance.update();
		 *	vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);
		 * ~~~~~
		 *
		 * @return Descriptor to be used in vkCmdBindDescriptorSets.
		 */
		virtual VkDescriptorSet update();
		
	protected:
		friend class RenderableMesh;	// For responding to pipeline state events

		bool initInternal(RenderService& renderService, utility::ErrorState& errorState);

		void onUniformCreated();
		void onSamplerChanged(int imageStartIndex, SamplerInstance& samplerInstance);
		void onStorageUniformChanged(int storageBufferIndex, StorageUniformInstance& storageUniformInstance);

		void rebuildUBO(UniformBufferObject& ubo, UniformStructInstance* overrideStruct);
		bool rebuildSSBO(StorageUniformBufferObject& ssbo, StorageUniformStructInstance* overrideStruct, uint hboIndex, utility::ErrorState& errorState);

		void updateStorageUniforms(const DescriptorSet& descriptorSet);

		void updateSamplers(const DescriptorSet& descriptorSet);
		bool initSamplers(utility::ErrorState& errorState);
		void addImageInfo(const Texture2D& texture2D, VkSampler sampler);

		SamplerInstance* getOrCreateSamplerInternal(const std::string& name);

	protected:
		VkDevice								mDevice = nullptr;						// Vulkan device
		RenderService*							mRenderService = nullptr;				// RenderService	

		DescriptorSetCache*						mDescriptorSetCache;					// Cache used to acquire Vulkan DescriptorSets on each update
		std::vector<UniformBufferObject>		mUniformBufferObjects;					// List of all UBO instances

		std::vector<StorageUniformBufferObject>	mStorageBufferObjects;					// List of all HBO instances
		std::vector<VkWriteDescriptorSet>		mStorageWriteDescriptorSets;			// List of storage unform descriptors, used to update Descriptor Sets
		std::vector<VkDescriptorBufferInfo>		mStorageDescriptors;					// List of storage buffers, used to update Descriptor Sets.

		std::vector<VkWriteDescriptorSet>		mSamplerWriteDescriptorSets;			// List of sampler descriptors, used to update Descriptor Sets
		std::vector<VkDescriptorImageInfo>		mSamplerWriteDescriptors;				// List of sampler images, used to update Descriptor Sets.
		bool									mUniformsCreated = false;				// Set when a uniform instance is created in between draws
	};

	/**
	 * To draw an object with a Material, you need to use a MaterialInstance. MaterialInstance contains the
	 * runtime data for drawing a Material. MaterialInstance is intended to be used as a property in Components. 
	 * It needs to be initialized based on a MaterialInstanceResource object to fill it's runtime data. 
	 * init() needs to be called from the ComponentInstance's init() function. Before drawing, make sure to call 
	 * update() to update the uniforms and samplers. A descriptor will be returned that can be used to issue
	 * the Vulkan draw call.
	 *
	 * Multiple MaterialInstances can share a single Material and a single MaterialInstance can override Material 
	 * properties on a per-instance basis. This means that you can set uniform or texture data on Material level, 
	 * which means that, as long as the property isn't overridden, you will set it for all MaterialInstances in one
	 * go. If you set a property on MaterialInstance level, you will set it only for that MaterialInstance.
	 * 
	 * It is also possible to set uniform or texture state on a single MaterialInstance multiple times per frame. 
	 * When multiple draws are performed with the frame, the state at the point of draw will be used.
	 *
	 * Performance note: changing the Depth mode or Blend mode frequently on a single MaterialInstance is not recommended,
	 * as it requires a rebuild of the entire GPU pipeline. If changing it per frame is required, consider using multiple
	 * MaterialInstance objects and switch between them instead.
	 */
	class NAPAPI MaterialInstance : public BaseMaterialInstance
	{
		RTTI_ENABLE(BaseMaterialInstance)
	public:

		/**
		 * Initializes all runtime structures for MaterialInstance.
		 */
		bool init(RenderService& renderService, MaterialInstanceResource& resource, utility::ErrorState& errorState);

		/**
		* @return material that this instance is overriding.
		*/
		Material& getMaterial();

		/**
		 * @return material that this instance is overriding
		 */
		const Material& getMaterial() const;

		/**
		 * @return base material that this instance is overriding
		 */
		virtual BaseMaterial* getBaseMaterial() override;

		/**
		 * @return base material that this instance is overriding
		 */
		virtual const BaseMaterial* getBaseMaterial() const override;

		/**
		 * @return base material instance resource
		 */
		virtual const BaseMaterialInstanceResource* getResource() const override;

		/**
		* @return If blend mode was overridden for this material, returns blend mode, otherwise material's blendmode.
		*/
		EBlendMode getBlendMode() const;

		/**
		 * Sets the blend mode that is used when rendering an object with this material. Note that frequently changing
		 * the blend mode on the same MaterialInstance requires a rebuild of the underlying GPU pipeline. When frequent 
		 * changes are required, it is recommended to use multiple MaterialInstance objects instead.
		 * @param blendMode the new blend mode
		 */
		void setBlendMode(EBlendMode blendMode);

		 /**
		 * Sets the depth mode that is used when rendering an object with this material. Note that frequently changing
		 * the depth mode on the same MaterialInstance requires a rebuild of the underlying GPU pipeline. When frequent
		 * changes are required, it is recommended to use multiple MaterialInstance objects instead.
		 * @param depthMode the new depth mode
		 */
		void setDepthMode(EDepthMode depthMode);

		/**
		* @return If depth mode was overridden for this material, returns depth mode, otherwise material's depthmode.
		*/
		EDepthMode getDepthMode() const;

	private:
		MaterialInstanceResource*				mResource;								// Resource this instance is associated with
	};

	/**
	 * To draw an object with a Material, you need to use a MaterialInstance. MaterialInstance contains the
	 * runtime data for drawing a Material. MaterialInstance is intended to be used as a property in Components.
	 * It needs to be initialized based on a MaterialInstanceResource object to fill it's runtime data.
	 * init() needs to be called from the ComponentInstance's init() function. Before drawing, make sure to call
	 * update() to update the uniforms and samplers. A descriptor will be returned that can be used to issue
	 * the Vulkan draw call.
	 *
	 * Multiple MaterialInstances can share a single Material and a single MaterialInstance can override Material
	 * properties on a per-instance basis. This means that you can set uniform or texture data on Material level,
	 * which means that, as long as the property isn't overridden, you will set it for all MaterialInstances in one
	 * go. If you set a property on MaterialInstance level, you will set it only for that MaterialInstance.
	 *
	 * It is also possible to set uniform or texture state on a single MaterialInstance multiple times per frame.
	 * When multiple draws are performed with the frame, the state at the point of draw will be used.
	 *
	 * Performance note: changing the Depth mode or Blend mode frequently on a single MaterialInstance is not recommended,
	 * as it requires a rebuild of the entire GPU pipeline. If changing it per frame is required, consider using multiple
	 * MaterialInstance objects and switch between them instead.
	 */
	class NAPAPI ComputeMaterialInstance : public BaseMaterialInstance
	{
		RTTI_ENABLE(BaseMaterialInstance)
	public:

		/**
		 * Initializes all runtime structures for ComputeMaterialInstance.
		 */
		bool init(RenderService& renderService, ComputeMaterialInstanceResource& resource, utility::ErrorState& errorState);

		/**
		* @return material that this instance is overriding.
		*/
		ComputeMaterial& getComputeMaterial();

		/**
		 * @return material that this instance is overriding
		 */
		const ComputeMaterial& getComputeMaterial() const;

		/**
		 * @return base material that this instance is overriding
		 */
		virtual BaseMaterial* getBaseMaterial() override;

		/**
		 * @return base material that this instance is overriding
		 */
		virtual const BaseMaterial* getBaseMaterial() const override;

		/**
		 * @return base material instance resource
		 */
		virtual const BaseMaterialInstanceResource* getResource() const override;

	private:
		ComputeMaterialInstanceResource* mResource;								// Resource this instance is associated with
	};
	
	template<class T>
	T* BaseMaterialInstance::getOrCreateSampler(const std::string& name)
	{
		return rtti_cast<T>(getOrCreateSamplerInternal(name));
	}
}
