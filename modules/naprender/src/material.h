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
	class Renderer;
	struct DescriptorSet;
	class DescriptorSetAllocator;
	class UniformLeafInstance;

	/**
	 * Blend mode for Materials.
	 */
	enum class EBlendMode : int
	{
		NotSet = 0,				///< Default value for MaterialInstances, means that the Material's blend mode is used instead
		Opaque,					///< Regular opaque, similar to (One, Zero) blend
		AlphaBlend,				///< Transparent object (SrcAlpha, InvSrcAlpha) blend
		Additive				///< Additive, (One, One) blend
	};

	/**
	 * Determines how the z-buffer is used for reading and writing.
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
	};

	using MaterialCreator = rtti::ObjectCreator<Material, RenderService>;
}
