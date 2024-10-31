/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resourceptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>
#include <nap/signalslot.h>
#include <rtti/typeinfo.h>

// Local includes
#include "uniformcontainer.h"
#include "descriptorsetcache.h"
#include "material.h"

namespace nap
{
	class Material;
	class RenderService;
	struct DescriptorSet;
	class DescriptorSetCache;

	namespace material
	{
		namespace instance
		{
			// RTTI get or create material function
			constexpr const char* getOrCreateMaterial = "getOrCreateMaterial";
		}
	}

	/**
	 * Base class of MaterialInstanceResource and ComputeMaterialInstanceResource
	 */
	class NAPAPI BaseMaterialInstanceResource
	{
		RTTI_ENABLE()
	public:
		std::vector<ResourcePtr<UniformStruct>>		mUniforms;										///< Property: "Uniforms" uniform structs to override
		std::vector<ResourcePtr<Sampler>>			mSamplers;										///< Property: "Samplers" samplers that you're overriding
		std::vector<ResourcePtr<BufferBinding>>		mBuffers;										///< Property: "Buffers" buffer bindings to override
		std::vector<ResourcePtr<ShaderConstant>>	mConstants;										///< Property: "Constants" shader constants to override

		/**
		 * @return material property
		 */
		rttr::property getMaterialProperty() const;

		/**
		 * @return material property name
		 */
		const std::string& getMaterialPropertyName() const;

	protected:
		BaseMaterialInstanceResource(std::string&& materialPropertyName);

	protected:
		std::string mMaterialPropertyName;
	};

	/**
	 * MaterialInstanceResource is the 'resource' or 'data' counterpart of MaterialInstance, intended to be used
	 * as fields in ComponentResources. The object must be passed to MaterialInstance's init() function.
	 */
	class NAPAPI MaterialInstanceResource : public BaseMaterialInstanceResource
	{
		RTTI_ENABLE(BaseMaterialInstanceResource)
	public:
		static constexpr const char* matProperty = "Material";

		MaterialInstanceResource() :
			BaseMaterialInstanceResource(matProperty)	{}

		ResourcePtr<Material>						mMaterial;											///< Property: "Material" Source material
		EBlendMode									mBlendMode = EBlendMode::NotSet;					///< Property: "BlendMode" Blend mode override. Uses source material blend mode by default
		EDepthMode									mDepthMode = EDepthMode::NotSet;					///< Property: "DepthMode" Depth mode override. Uses source material depth mode by default
	};

	/**
	 * ComputeMaterialInstanceResource is the 'resource' or 'data' counterpart of ComputeMaterialInstance, intended to be
	 * used as fields in ComponentResources. The object must be passed to ComputeMaterialInstance's init() function.
	 */
	class NAPAPI ComputeMaterialInstanceResource : public BaseMaterialInstanceResource
	{
		RTTI_ENABLE(BaseMaterialInstanceResource)
	public:
		static constexpr const char* matProperty = "ComputeMaterial";

		ComputeMaterialInstanceResource() :
			BaseMaterialInstanceResource(matProperty)	{}

		ResourcePtr<ComputeMaterial>				mComputeMaterial;									///< Property: "ComputeMaterial" source material
	};

	/**
	 * Base class of MaterialInstance and ComputeMaterialInstance
	 */
	class NAPAPI BaseMaterialInstance : public UniformContainer
	{
		RTTI_ENABLE(UniformContainer)
		friend class RenderService;
	public:
		/**
		 * Gets or creates a uniform struct (ubo) for this material instance.
		 * This means that the uniform returned is only applicable to this instance.
		 * In order to change a uniform so that its value is shared among MaterialInstances, use getMaterial().getUniform().
		 *
		 * @param name: the name of the uniform struct (ubo) as declared in the shader.
		 * @return uniform that was found or created, nullptr if not available.
		 */
		virtual UniformStructInstance* getOrCreateUniform(const std::string& name);

		/**
		 * Gets or creates a nap::BufferBindingInstance of type T for this material.
		 * The binding can be used to set the buffer of type T at runtime.
		 * The returned binding is only applicable to this instance.
		 * This function will assert if the name of the binding does not match the type that you are trying to create.
		 *
		 * ~~~~~{.cpp}
		 * material_instance->getOrCreateBinding<nap::BufferBindingVec4Instance>("inBinding");
		 * ~~~~~
		 *
		 * @param name: the name of the buffer binding as declared in the shader.
		 * @return buffer binding that was found or created, nullptr if not available.
		 */
		template<class T>
		T* getOrCreateBuffer(const std::string& name);

		/**
		 * Gets or creates a buffer binding instance for this material.
		 * The binding can be used to set any buffer of type 'BufferBindingInstance' at runtime.
		 * The returned buffer binding is only applicable to this instance.
		 *
		 * @param name: the name of the buffer binding as declared in the shader.
		 * @return buffer binding that was found or created, nullptr if not available.
		 */
		BufferBindingInstance* getOrCreateBuffer(const std::string& name) { return getOrCreateBufferInternal(name); }

		/**
		 * Gets or creates a nap::SamplerInstance of type T for this material instance.
		 * This means that the sampler returned is only applicable to this instance.
		 * In order to change a sampler so that its value is shared among MaterialInstances, use getMaterial().findSampler().
		 *
		 * ~~~~~{.cpp}
		 * material_instance->getOrCreateSampler<nap::Sampler2DInstance>("inTexture");
		 * ~~~~~
		 *
		 * @param name: the name of the sampler declared in the shader.
		 * @return nap::SamplerInstance of type T, nullptr when sampler declaration doesn't exist or of incorrect type
		 */
		template<class T>
		T* getOrCreateSampler(const std::string& name);

		/**
		 * Gets or creates a nap::SamplerInstance for this material, which can be set at runtime.
		 * The returned sampler is only applicable to this instance.
		 * In order to change a sampler so that its value is shared among MaterialInstances, use getMaterial().findSampler().
		 *
		 * @param name: the name of the sampler declared in the shader.
		 * @return the sampler instance, nullptr when sampler declaration doesn't exist
		 */
		SamplerInstance* getOrCreateSampler(const std::string& name)		{ return getOrCreateSamplerInternal(name, nullptr); }

		/**
		 * Get or creates a nap::SamplerInstance of type T for this material, which can be set at runtime.
		 * The instance is initialized against the provided resource and only applicable to this instance.
		 *
		 * Note that the resource type must match the instance type! The function asserts otherwise.
		 * In order to change a sampler so that its value is shared among MaterialInstances, use getMaterial().findSampler().
		 *
		 * ~~~~~{.cpp}
		 * material_instance->getOrCreateSampler<nap::Sampler2DInstance>(samplerResource);
		 * ~~~~~
		 * 
		 * @param resource: the resource to get the instance for
		 * @return the sampler instance of type T, nullptr when sampler declaration doesn't exist
		 */
		template<class T>
		T* getOrCreateSampler(const Sampler& resource);

		/**
		 * Get or creates a nap::SamplerInstance for this material, which can be set at runtime.
		 * The instance is initialized against the provided resource and only applicable to this instance.
		 *
		 * Note that the resource type must match the instance type! The function asserts otherwise.
		 * In order to change a sampler so that its value is shared among MaterialInstances, use getMaterial().findSampler().
		 *
		 * @param resource: the resource to get the instance for
		 * @return the sampler instance, nullptr when sampler declaration doesn't exist or of incorrect type
		 */
		SamplerInstance* getOrCreateSampler(const Sampler& resource)		{ return getOrCreateSamplerInternal(resource.mName, &resource); }

		/**
		 * @return base material that this instance is overriding
		 */
		BaseMaterial* getMaterial()											{ assert(mMaterial != nullptr); return mMaterial; }

		/**
		 * @return base material that this instance is overriding
		 */
		const BaseMaterial* getMaterial() const								{ assert(mMaterial != nullptr); return mMaterial; }

		/**
		 * This must be called before each draw. It will push the current uniform and sampler data into memory
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
		virtual const DescriptorSet& update();
		
	protected:
		friend class RenderableMesh;	// For responding to pipeline state events

		bool initInternal(RenderService& renderService, BaseMaterial& material, BaseMaterialInstanceResource& instanceResource, utility::ErrorState& errorState);
		void rebuildUBO(UniformBufferObject& ubo, UniformStructInstance* overrideStruct);

		void onUniformCreated();
		void onSamplerChanged(int imageStartIndex, SamplerInstance& samplerInstance, int imageArrayIndex);
		void onBufferChanged(int storageBufferIndex, BufferBindingInstance& bindingInstance);

		void updateBuffers(const DescriptorSet& descriptorSet);
		bool initBuffers(BaseMaterialInstanceResource& resource, utility::ErrorState& errorState);

		void updateSamplers(const DescriptorSet& descriptorSet);
		bool initSamplers(BaseMaterialInstanceResource& resource, utility::ErrorState& errorState);
		void addImageInfo(const Texture& texture, VkSampler sampler);

		bool initConstants(BaseMaterialInstanceResource& resource, utility::ErrorState& errorState);

		BufferBindingInstance* getOrCreateBufferInternal(const std::string& name);
		SamplerInstance* getOrCreateSamplerInternal(const std::string& name, const Sampler* sampler);

		/**
		 * @return a map that groups shader constant ids by shader stage. Used for creating vulkan pipelines.
		 */
		const ShaderStageConstantMap& getShaderStageConstantMap() const		{ return mShaderStageConstantMap; }

		/**
		 * @return the shader constant hash for quick distinction of constant data in material instances
		 */
		ShaderConstantHash getConstantHash() const							{ return mConstantHash; }

	protected:
		VkDevice								mDevice = nullptr;						// Vulkan device
		RenderService*							mRenderService = nullptr;				// RenderService
		BaseMaterial*							mMaterial = nullptr;					// Material
		BaseMaterialInstanceResource*			mResource = nullptr;					// Material Instance Resource

		DescriptorSetCache*						mDescriptorSetCache;					// Cache used to acquire Vulkan DescriptorSets on each update
		std::vector<UniformBufferObject>		mUniformBufferObjects;					// List of all UBO instances

		std::vector<VkWriteDescriptorSet>		mStorageWriteDescriptorSets;			// List of storage storage descriptors, used to update Descriptor Sets
		std::vector<VkDescriptorBufferInfo>		mStorageDescriptors;					// List of storage buffers, used to update Descriptor Sets.

		std::vector<VkWriteDescriptorSet>		mSamplerWriteDescriptorSets;			// List of sampler descriptors, used to update Descriptor Sets
		std::vector<VkDescriptorImageInfo>		mSamplerDescriptors;					// List of sampler images, used to update Descriptor Sets.

		ShaderStageConstantMap					mShaderStageConstantMap;				// Reference of all shader constants per shader stage, generated on materialinstance init
		ShaderConstantHash						mConstantHash;							// Shader constant hash used to create a pipeline key

		bool									mUniformsCreated = false;				// Set when a uniform instance is created in between draws
	};

	/**
	 * To draw an object with a Material, you must use a MaterialInstance. MaterialInstance contains the
	 * runtime data for drawing a Material. MaterialInstance is intended to be used as a property in Components. 
	 * It must be initialized based on a MaterialInstanceResource object to fill its runtime data. 
	 * init() must be called from the ComponentInstance's init() function. Before drawing, make sure to call 
	 * update() to update the uniforms and samplers. A descriptorset will be returned that can be used to issue
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
	 * Note that there is no implicit synchronization of access to shader resources bound to storage and regular uniforms
	 * between render passes. Therefore, it is currently not recommended to write to storage buffers inside vertex
	 * and/or fragment shaders over consecutive render passes within a single frame.
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
		Material& getMaterial()																{ return static_cast<Material&>(*BaseMaterialInstance::getMaterial()); }

		/**
		 * @return material that this instance is overriding
		 */
		const Material& getMaterial() const													{ return static_cast<const Material&>(*BaseMaterialInstance::getMaterial()); }

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
	 * To run a compute shader, you must use a ComputeMaterialInstance. This material contains the runtime resources that
	 * are bound to shader variable inputs in your compute shader. ComputeMaterialInstance is intended to be used as a
	 * property in ComputeComponents. It must be initialized based on a ComputeMaterialInstanceResource object to fill
	 * its runtime data. init() must be called from the ComponentInstance's init() function. Before drawing, make sure
	 * to call update() to update the uniforms and samplers. A descriptorset will be returned that can be used to issue
	 * the Vulkan draw call.
	 *
	 * Multiple ComputeMaterialInstances can share a single ComputeMaterial and a single ComputeMaterialInstance can
	 * override ComputeMaterial properties on a per-instance basis. This means that you can set uniform, buffer or
	 * texture data on ComputeMaterial level, which means that, as long as the property isn't overridden, you will set
	 * it for all ComputeMaterialInstances in one go. If you set a property on ComputeMaterialInstance level, you will
	 * set it only for that ComputeMaterialInstance.
	 *
	 * It is also possible to set uniform, buffer or texture state on a single MaterialInstance multiple times per frame.
	 * When multiple draws are performed with the frame, the state at the point of draw will be used.
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
		ComputeMaterial& getMaterial()													{ return static_cast<ComputeMaterial&>(*BaseMaterialInstance::getMaterial()); }

		/**
		 * @return material that this instance is overriding
		 */
		const ComputeMaterial& getMaterial() const										{ return static_cast<const ComputeMaterial&>(*BaseMaterialInstance::getMaterial()); }

		/**
		 * @return the workgroup size
		 */
		glm::uvec3 getWorkGroupSize() const;

	private:
		ComputeMaterialInstanceResource*		mResource;								// Resource this instance is associated with
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<class T>
	T* BaseMaterialInstance::getOrCreateBuffer(const std::string& name)
	{
		return rtti_cast<T>(getOrCreateBufferInternal(name));
	}
	
	template<class T>
	T* BaseMaterialInstance::getOrCreateSampler(const std::string& name)
	{
		return rtti_cast<T>(getOrCreateSamplerInternal(name, nullptr));
	}

	template<class T>
	T* BaseMaterialInstance::getOrCreateSampler(const Sampler& resource)
	{
		return rtti_cast<T>(getOrCreateSamplerInternal(resource.mName, &resource));
	}
}
