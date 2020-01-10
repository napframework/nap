#pragma once

// External includes
#include <nap/resourceptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>
#include <nap/signalslot.h>

// Local includes
#include "shader.h"
#include "imagefromfile.h"
#include "ngpumesh.h"
#include "uniformcontainer.h"
#include "rtti/factory.h"

namespace nap
{
	class Material;
	class MaterialInstance;
	class UniformLeafInstance;
	class Renderer;
	struct DescriptorSet;
	class DescriptorSetAllocator;

	/**
	 * Blend mode for Materials.
	 */
	enum class EBlendMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend mode is used instead
		Opaque,					///< Regular opaque, similar to (One, Zero) blend
		AlphaBlend,				///< Transparant object (SrcAlpha, InvSrcAlpha) blend
		Additive				///< Additive, (One, One) blend
	};

	/**
	 * Determines how to z-buffer is used for reading and writing.
	 * When inheriting from blend mode
	 */
	enum class EDepthMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend is used instead
		InheritFromBlendMode,	///< Transparent objects do not write depth, but do read depth. Opaque objects read and write depth.
		ReadWrite,				///< Read and write depth
		ReadOnly,				///< Only read depth
		WriteOnly,				///< Only write depth
		NoReadWrite				///< Neither read or write depth
	};


	/**
	 * MaterialInstanceResource is the 'resource' or 'data' counterpart of MaterialInstance, intended to be used 
	 * as fields in ComponentResources. The object needs to be passed to MaterialInstance's init() function.
	 */
	class NAPAPI MaterialInstanceResource
	{
	public:
		std::vector<ResourcePtr<UniformStruct>>		mUniforms;										///< Property: "Uniforms" that you're overriding
		std::vector<ResourcePtr<Sampler>>			mSamplers;										///< Property: 
		ResourcePtr<Material>						mMaterial;										///< Property: "Material" that you're overriding uniforms from
		EBlendMode									mBlendMode = EBlendMode::NotSet;				///< Property: "BlendMode" Blend mode override. By default uses material blend mode
		EDepthMode									mDepthMode = EDepthMode::NotSet;				///< Property: "DepthMode" Depth mode override. By default uses material depth mode
	};

	class UniformBufferObject
	{
	public:
		using UniformList = std::vector<const UniformLeafInstance*>;

		UniformBufferObject(const opengl::UniformBufferObjectDeclaration& declaration) :
			mDeclaration(&declaration)
		{
		}

		const opengl::UniformBufferObjectDeclaration*	mDeclaration;
		UniformList										mUniforms;
	};

	/**
	 * Run time version of a nap::Material. The instance holds overloaded material properties such as
	 * the uniform shader variables. Use the getOrCreateUniform() functions to create and get access
	 * to the underlying shader uniforms. The instance is intended to be used as a property in Components. 
	 * It needs to be initialized based on a MaterialInstanceResource object to fill it's
	 * runtime data. init() needs to be called from the ComponentInstance's init() function.
	 */
	class NAPAPI MaterialInstance : public UniformContainer
	{
		RTTI_ENABLE(UniformContainer)
	public:

		/**
		 * For each uniform in mUniforms, creates a mapping.
		 */
		bool init(RenderService& renderService, MaterialInstanceResource& resource, utility::ErrorState& errorState);

		/**
		* @return material that this instance is overriding.
		*/
		Material* getMaterial();

		/**
		* @return If blend mode was overridden for this material, returns blend mode, otherwise material's blendmode.
		*/
		EBlendMode getBlendMode() const;

		/**
		 * Sets the blend mode that is used when rendering an object with this material
		 * @param blendMode the new blend mode
		 */
		void setBlendMode(EBlendMode blendMode);

		/**
		 *	Sets the depth mode used when rendering an object with this material
		 * @param depthMode the new depth mode
		 */
		void setDepthMode(EDepthMode depthMode);

		/**
		* @return If depth mode was overridden for this material, returns depth mode, otherwise material's depthmode.
		*/
		EDepthMode getDepthMode() const;

		/**
		 * Get a uniform for this material instance. This means that the uniform returned is only applicable
		 * to this instance. In order to change a uniform so that it's value is shared among materials, use
		 * getMaterial().getUniforms().getUniform(). This function will assert if the name of the uniform does not 
		 * match the type that you are trying to create.
		 * 
		 * regular uniform get example:
		 * nap::Uniform* mesh_color = material.getOrCreateUniform<nap::UniformVec3>("meshColor");
		 *
		 * uniform array get example:
		 * nap::Uniform* textures = material.getOrCreateUniform<nap::UniformTextureArray>("textures");
		 *
		 * uniform member from struct example:
		 * nap::Uniform* intensity = material.getOrCreateUniform<nap::UniformFloat>("light.intensity");
		 *
		 * uniform member from struct array example:
		 * nap::Uniform* intensity = material.getOrCreateUniform<nap::UniformFloat>("lights[0].intensity");
		 *
		 * @param name: the name of the uniform as it is in the shader.
		 * @return reference to the uniform that was found or created.
		 */
		UniformStructInstance& getOrCreateUniform(const std::string& name);

		template<class T>
		T& getOrCreateSampler(const std::string& name);

		/**
		 * Uploads all uniforms stored in this material to the GPU. Call this after binding!
		 * Only call this after binding the material otherwise the outcome of this call is uncertain.
		 * This applies to the uniforms in the instance that are overridden as for the uniforms in the underlying material.
		 */
		VkDescriptorSet update();

		Signal<const MaterialInstance&, RenderService&> pipelineStateChanged;

	private:
		friend class RenderService;

		void onUniformCreated();
		void onSamplerChanged(int imageStartIndex, SamplerInstance& samplerInstance);
		void rebuildUBO(UniformBufferObject& ubo, UniformStructInstance* overrideStruct);

		void updateUniforms(const DescriptorSet& descriptorSet);
		void updateSamplers(const DescriptorSet& descriptorSet);
		bool initSamplers(utility::ErrorState& errorState);

		SamplerInstance& getOrCreateSamplerInternal(const std::string& name);

	private:
		//. Resource this instance is associated with
		MaterialInstanceResource*				mResource;
		VkDevice								mDevice = nullptr;
		RenderService*							mRenderService = nullptr;
		DescriptorSetAllocator*					mDescriptorSetAllocator = nullptr;
		std::vector<UniformBufferObject>		mUniformBufferObjects;
		std::vector<SamplerInstance*>			mSamplers;
		std::vector<VkWriteDescriptorSet>		mSamplerDescriptors;
		std::vector<VkDescriptorImageInfo>		mSamplerImages;
		bool									mUniformsDirty = false;
	};


	/**
	 * A Material is a resource that acts as an interface to a shader.
	 * It contains default mappings for how mesh vertex attributes are bound to a shader vertex attributes.
	 * It also holds the uniform values for all the uniforms present in the shader. If a uniform is updated 
	 * on the material, all the objects that use this material will use that value. To change uniform values
	 * per object, set uniform values on MaterialInstances.
	 */
	class NAPAPI Material : public Resource, public UniformContainer
	{
		RTTI_ENABLE(Resource)
	public:
		Material() = default;
		Material(RenderService& renderService);

		/**
		 * Binding between mesh vertex attr and shader vertex attr
		*/
		struct VertexAttributeBinding
		{
			VertexAttributeBinding() = default;

			VertexAttributeBinding(const std::string& meshAttributeID, const std::string& shaderAttributeID) :
				mMeshAttributeID(meshAttributeID),
				mShaderAttributeID(shaderAttributeID)  {}

			std::string mMeshAttributeID;
			std::string mShaderAttributeID;
		};

		/**
		* Creates mappings for uniform and vertex attrs.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Utility for getting the shader resource
		 * @return the link as a shader resource, nullptr if not linked
		 */
		Shader* getShader() const				{ return mShader.get(); }

		/**
		* @return Blending mode for this material
		*/
		EBlendMode getBlendMode() const			{ assert(mBlendMode != EBlendMode::NotSet); return mBlendMode; }

		/**
		* @return Depth mode mode for this material
		*/
		EDepthMode getDepthMode() const			{ assert(mDepthMode != EDepthMode::NotSet); return mDepthMode; }

		/**
		* Finds the mesh/shader attribute binding based on the shader attribute ID.
		* @param shaderAttributeID: ID of the shader vertex attribute.
		*/
		const VertexAttributeBinding* findVertexAttributeBinding(const std::string& shaderAttributeID) const;

		/**
		* @return Returns a mapping with default values for mesh attribute IDs an shader attribute IDs.
		*/
		static const std::vector<VertexAttributeBinding>& sGetDefaultVertexAttributeBindings();

		VkDescriptorSetLayout getDescriptorSetLayout() const { return mDescriptorSetLayout; }

		Renderer& getRenderer() { return *mRenderer; }

	public:
		std::vector<ResourcePtr<UniformStruct>>		mUniforms;											///< Property: 'Uniforms' Static uniforms (as read from file, or as set in code before calling init())
		std::vector<ResourcePtr<Sampler>>			mSamplers;											///< Property: 
		std::vector<VertexAttributeBinding>			mVertexAttributeBindings;							///< Property: 'VertexAttributeBindings' Optional, mapping from mesh vertex attr to shader vertex attr
		ResourcePtr<Shader>							mShader = nullptr;									///< Property: 'Shader' The shader that this material is using
		EBlendMode									mBlendMode = EBlendMode::Opaque;					///< Property: 'BlendMode' Optional, blend mode for this material
		EDepthMode									mDepthMode = EDepthMode::InheritFromBlendMode;		///< Property: 'DepthMode' Optional, determines how the Z buffer is used

	private:
		using UniformStructMap = std::unordered_map<std::string, std::unique_ptr<UniformStruct>>;
		using UniformStructArrayMap = std::unordered_map<std::string, std::unique_ptr<UniformStructArray>>;

		Renderer*									mRenderer = nullptr;
		VkDescriptorSetLayout						mDescriptorSetLayout = nullptr;
	};

	template<class T>
	T& MaterialInstance::getOrCreateSampler(const std::string& name)
	{
		return *rtti_cast<T>(&getOrCreateSamplerInternal(name));
	}

	using MaterialCreator = rtti::ObjectCreator<Material, RenderService>;
}
