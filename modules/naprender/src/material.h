/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resourceptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>

// Local includes
#include "uniformcontainer.h"
#include "materialcommon.h"
#include "shader.h"

namespace nap
{
	// Forward Declares
	struct DescriptorSet;
	class DescriptorSetCache;
	class Core;

	class BaseMaterial : public Resource, public UniformContainer
	{
		RTTI_ENABLE(Resource)
	public:
		/** 
		* Base constructor associated with a material
		*/
		BaseMaterial(Core& core);
		virtual ~BaseMaterial() = default;

		std::vector<ResourcePtr<UniformStruct>>			mUniforms;										///< Property: 'Uniforms' Static uniforms (as read from file, or as set in code before calling init())
		std::vector<ResourcePtr<StorageUniformStruct>>	mStorageUniforms;								///< Property: 'StorageUniforms' Static uniforms (as read from file, or as set in code before calling init())
		std::vector<ResourcePtr<Sampler>>				mSamplers;										///< Property: 'Samplers' Static samplers (as read from file, or as set in code before calling init())

		/**
		 * @return The underlying base shader
		 */
		virtual const BaseShader* getBaseShader() const = 0;

	protected:
		bool rebuild(const BaseShader& shader, utility::ErrorState& errorState);

	private:
		using UniformStructMap = std::unordered_map<std::string, std::unique_ptr<UniformStruct>>;
		using UniformStructArrayMap = std::unordered_map<std::string, std::unique_ptr<UniformStructArray>>;

		RenderService* mRenderService = nullptr;
	};

	/**
	 * Resource that acts as the main interface to a shader. Controls how vertex buffers are bound to shader inputs.
	 * It also creates and holds a set of uniform struct instances, matching those exposed by the shader.
	 * If a uniform exposed by this material is updated, all the objects rendered using this material will use 
	 * that same value, unless overridden by a nap::MaterialInstance. 
	 */
	class NAPAPI Material : public BaseMaterial
	{
		RTTI_ENABLE(BaseMaterial)
	public:
		/**
		 * @param core the core instance
		 */
		Material(Core& core);

		/**
		 * Binds a mesh vertex buffer to a shader input.
		 */
		struct VertexAttributeBinding
		{
			/**
			 * @param meshAttributeID the mesh vertex buffer name
			 * @param shaderAttributeID the shader vertex buffer name
			 */
			VertexAttributeBinding(const std::string& meshAttributeID, const std::string& shaderAttributeID);

			// Default constructor
			VertexAttributeBinding() = default;

			std::string mMeshAttributeID;				///< mesh vertex buffer name
			std::string mShaderAttributeID;				///< shader vertex buffer name
		};

		/**
		 * Initializes the material. 
		 * Validates and converts all the declared uniform values into instances and sets up the vertex buffer bindings.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return The underlying shader
		 */
		const Shader& getShader() const						{ assert(mShader != nullptr); return *mShader; }

		/**
		 * @return The underlying base shader
		 */
		virtual const BaseShader* getBaseShader() const		{ assert(mShader != nullptr); return static_cast<BaseShader*>(mShader.get()); }

		/**
		 * Returns the current blend mode.
		 * Shared by all objects rendered with this material, unless overridden in a nap::MaterialInstance.
		 * @return Active blend mode
		 */
		EBlendMode getBlendMode() const						{ assert(mBlendMode != EBlendMode::NotSet); return mBlendMode; }

		/**
		 * Sets the blend mode to use. 
		 * Shared by all objects rendered with this material, unless overridden in a nap::MaterialInstance.
		 * @param blendMode new blend mode to use
		 */
		void setBlendMode(EBlendMode blendMode)				{ mBlendMode = blendMode; }

		/**
		 * Returns the current depth mode.
		 * Shared by all objects rendered with this material, unless overridden in a nap::MaterialInstance.
		 * @return Depth mode
		 */
		EDepthMode getDepthMode() const						{ assert(mDepthMode != EDepthMode::NotSet); return mDepthMode; }

		/**
		 * Sets the depth mode to use.
		 * Shared by all objects rendered with this material, unless overridden in a nap::MaterialInstance.
		 * @param depthMode new depth mode to use.
		 */
		void setDepthMode(EDepthMode depthMode)				{ mDepthMode = depthMode; }

		/**
		 * Finds the mesh / shader attribute binding based on the given shader attribute ID.
		 * @param shaderAttributeID: ID of the shader vertex attribute.
		 */
		const VertexAttributeBinding* findVertexAttributeBinding(const std::string& shaderAttributeID) const;

		/**
		 * @return Returns a mapping with default values for mesh to shader vertex attribute IDs.
		 */
		static const std::vector<VertexAttributeBinding>& sGetDefaultVertexAttributeBindings();

	public:
		std::vector<VertexAttributeBinding>			mVertexAttributeBindings;							///< Property: 'VertexAttributeBindings' Optional, mapping from mesh vertex attr to shader vertex attr
		ResourcePtr<Shader>							mShader = nullptr;									///< Property: 'Shader' The shader that this material is using
		EBlendMode									mBlendMode = EBlendMode::Opaque;					///< Property: 'BlendMode' Optional, blend mode for this material
		EDepthMode									mDepthMode = EDepthMode::InheritFromBlendMode;		///< Property: 'DepthMode' Optional, determines how the Z buffer is used
	};


	/**
	 * Resource that acts as the main interface to a shader. Controls how vertex buffers are bound to shader inputs.
	 * It also creates and holds a set of uniform struct instances, matching those exposed by the shader.
	 * If a uniform exposed by this material is updated, all the objects rendered using this material will use
	 * that same value, unless overridden by a nap::MaterialInstance.
	 */
	class NAPAPI ComputeMaterial : public BaseMaterial
	{
		RTTI_ENABLE(BaseMaterial)
	public:
		/**
		 * @param core the core instance
		 */
		ComputeMaterial(Core& core);

		/**
		 * Initializes the material.
		 * Validates and converts all the declared uniform values into instances and sets up the vertex buffer bindings.
		 * @param errorState contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return The underlying shader
		 */
		const ComputeShader& getShader() const				{ assert(mShader != nullptr); return *mShader; }

		/**
		 * @return The underlying base shader
		 */
		virtual const BaseShader* getBaseShader() const { assert(mShader != nullptr); return static_cast<BaseShader*>(mShader.get()); }

	public:
		ResourcePtr<ComputeShader>					mShader = nullptr;									///< Property: 'Shader' The shader that this material is using
	};
}
