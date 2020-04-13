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
	struct DescriptorSet;
	class DescriptorSetCache;
	class Core;

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
		Material(Core& core);

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
		const Shader& getShader() const			{ return *mShader; }

		/**
		* @return Blending mode for this material
		*/
		EBlendMode getBlendMode() const			{ assert(mBlendMode != EBlendMode::NotSet); return mBlendMode; }

		void setBlendMode(EBlendMode blendMode) { mBlendMode = blendMode; }

		/**
		* @return Depth mode mode for this material
		*/
		EDepthMode getDepthMode() const			{ assert(mDepthMode != EDepthMode::NotSet); return mDepthMode; }

		void setDepthMode(EDepthMode depthMode) { mDepthMode = depthMode; }

		/**
		* Finds the mesh/shader attribute binding based on the shader attribute ID.
		* @param shaderAttributeID: ID of the shader vertex attribute.
		*/
		const VertexAttributeBinding* findVertexAttributeBinding(const std::string& shaderAttributeID) const;

		/**
		* @return Returns a mapping with default values for mesh attribute IDs an shader attribute IDs.
		*/
		static const std::vector<VertexAttributeBinding>& sGetDefaultVertexAttributeBindings();

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

		RenderService*								mRenderService = nullptr;
	};
}
