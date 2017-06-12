#pragma once

// External includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/resourcelinkattribute.h>
#include <nap/objectptr.h>

// Local includes
#include "shaderresource.h"
#include "imageresource.h"
#include "uniforms.h"
#include "nmesh.h"

namespace nap
{
	class Material;

	/**
	* Blend mode for Materials.
	*/
	enum class EBlendMode
	{
		NotSet,					// Default value for MaterialInstances, means that the Material's blend mode is used instead

		Opaque,					// Regular opaque, similar to (One, Zero) blend
		AlphaBlend,				// Transparant object (SrcAlpha, InvSrcAlpha) blend
		Additive				// Additive, (One, One) blend
	};

	/**
	* Determines how to z-buffer is used for reading and writing.
	* When inheriting from blend mode
	*/
	enum class EDepthMode
	{
		NotSet,					// Default value for MaterialInstances, means that the Material's blend is used instead

		InheritFromBlendMode,	// Transparent objects do not write depth, but do read depth. Opaque objects read and write depth.
		ReadWrite,				// Read and write depth
		ReadOnly,				// Only read depth
		WriteOnly,				// Only write depth
		NoReadWrite				// Neither read or write depth
	};


	/**
	* Binds the Uniform data to the declaration from the shader. Together
	* they can be used to push the uniform.
	*/
	template<typename UNIFORM>
	struct UniformBinding
	{
		UniformBinding(std::unique_ptr<UNIFORM>&& uniform, const opengl::UniformDeclaration& declaration) :
			mUniform(std::move(uniform)),
			mDeclaration(&declaration)
		{
		}

		UniformBinding(UniformBinding&& other) :
			mUniform(std::move(other.mUniform)),
			mDeclaration(other.mDeclaration)
		{
		}

		UniformBinding& operator=(UniformBinding&& other)
		{
			mUniform = std::move(other.mUniform);
			mDeclaration = other.mDeclaration;
			return *this;
		}

		std::unique_ptr<UNIFORM> mUniform;
		const opengl::UniformDeclaration* mDeclaration;
	};

	using UniformTextureBindings = std::unordered_map<std::string, UniformBinding<UniformTexture>>;
	using UniformValueBindings = std::unordered_map<std::string, UniformBinding<UniformValue>>;


	/**
	 * Base class for both Material and MaterialInstance. Basic support for uniforms.
	 */
	class UniformContainer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		 * @return a uniform texture object that can be used to set a texture or value.
		 * If the uniform is not found, returns nullptr.
		 */
		template<typename T>
		T* findUniform(const std::string& name);

		/**
		 * @return a uniform object that can be used to set a texture or value.
		 * If the uniform is not found it will assert.
		 */
		template<typename T>
		T& getUniform(const std::string& name);

		/**
		 * @return All texture uniform bindings.
		 */
		const UniformTextureBindings& getUniformTextureBindings() { return mUniformTextureBindings; }

		/**
		 * @return All value uniform bindings.
		 */
		const UniformValueBindings& getUniformValueBindings() { return mUniformValueBindings; }

		std::vector<ObjectPtr<Uniform>>	mUniforms;					///< Static uniforms (as read from file, or as set in code before calling init())

	protected:
		/**
		 * Puts the uniform into either the texture or value mapping.
		 * @param uniform: the uniform to add. Ownership is transferred.
		 * @param declaration: the shader uniform declaration to bind the uniform to.
		 * @return reference to the newly added uniform.
		 */
		Uniform& AddUniform(std::unique_ptr<Uniform> uniform, const opengl::UniformDeclaration& declaration);

	private:
		UniformTextureBindings		mUniformTextureBindings;	///< Runtime map of texture uniforms (superset of texture uniforms in mUniforms due to default uniforms).
		UniformValueBindings		mUniformValueBindings;		///< Runtime map of value uniforms (superset of value uniforms in mUniforms due to default uniforms).
	};


	/**
	 * MaterialInstance holds uniform overloads of a Material.
	 */
	class MaterialInstance : public UniformContainer
	{
		RTTI_ENABLE(UniformContainer)
	public:
		/**
		 * For each uniform in mUniforms, creates a mapping.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *
		 */
		virtual const std::string getDisplayName() const { return "MaterialInstance"; }

		/**
		* @return material that this instance is overriding.
		*/
		Material* getMaterial();

		ObjectPtr<Material>	mMaterial;		///< Material that you're overriding uniforms from

		/**
		 * Get a uniform for this material instance. This means that the uniform returned is only applicable
		 * to this instance. In order to change a uniform so that it's value is shared among materials, use
		 * getMaterial().getUniform().
		 * This function will assert if the name of the uniform does not match the type that you are trying to 
		 * create.
		 * @param name: the name of the uniform as it is in the shader.
		 * @return reference to the uniform that was found or created.
		 */
		template<typename T>
		T& getOrCreateUniform(const std::string& name);

		/**
		* @return If blend mode was overridden for this material, returns blend mode, otherwise material's blendmode.
		*/
		EBlendMode getBlendMode() const;

		/**
		* @return If depth mode was overridden for this material, returns depth mode, otherwise material's depthmode.
		*/
		EDepthMode getDepthMode() const;

		EBlendMode mBlendMode = EBlendMode::NotSet;				///< Blend mode override. By default uses material blend mode
		EDepthMode mDepthMode = EDepthMode::NotSet;				///< Depth mode override. By default uses material depth mode

	private:
		Uniform& createUniform(const std::string& name);
	};


	/**
	* Material can be considered as the interface towards a shader.
	* It contains default mappings for how mesh vertex attributes are bound to a shader's vertex attributes.
	* It also holds the uniform values for all the uniforms present in the shader. If a uniform is updated 
	* on the material, all the objects that use this material will use that value. To change uniform values
	* per object, set uniform values on MaterialInstances.
	*/
	class Material : public UniformContainer
	{
		RTTI_ENABLE(UniformContainer)
	public:

		/**
		* Binding between mesh vertex attr and shader vertex attr
		*/
		struct VertexAttributeBinding
		{
			VertexAttributeBinding() = default;

			VertexAttributeBinding(const opengl::Mesh::VertexAttributeID& meshAttributeID, const opengl::Shader::VertexAttributeID& shaderAttributeID) :
				mMeshAttributeID(meshAttributeID),
				mShaderAttributeID(shaderAttributeID)
			{
			}
			opengl::Mesh::VertexAttributeID mMeshAttributeID;
			opengl::Shader::VertexAttributeID mShaderAttributeID;
		};

		// Default constructor
		Material() = default;

		/**
		* Creates mappings for uniform and vertex attrs.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return display name.
		*/
		virtual const std::string getDisplayName() const { return "Material"; }		// TODO

		/**
		* Binds the GLSL shader resource program
		*/
		void bind();

		/**
		* Unbinds the GLSL shader resource program
		*/
		void unbind();

		/**
		 * Utility for getting the shader resource
		 * @return the link as a shader resource, nullptr if not linked
		 */
		ShaderResource* getShader() const { return mShader.get(); }

		/**
		* @return Blending mode for this material
		*/
		EBlendMode getBlendMode() const { assert(mBlendMode != EBlendMode::NotSet); return mBlendMode; }

		/**
		* @return Depth mode mode for this material
		*/
		EDepthMode getDepthMode() const { assert(mDepthMode != EDepthMode::NotSet); return mDepthMode; }

		/**
		* Finds the mesh/shader attribute binding based on the shader attribute ID.
		* @param shaderAttributeID: ID of the shader vertex attribute.
		*/
		const VertexAttributeBinding* findVertexAttributeBinding(const opengl::Mesh::VertexAttributeID& shaderAttributeID) const;

		/**
		* @return Returns a mapping with default values for mesh attribute IDs an shader attribute IDs.
		*/
		static std::vector<VertexAttributeBinding>& getDefaultVertexAttributeBindings();

	public:
		std::vector<VertexAttributeBinding>		mVertexAttributeBindings;							///< Mapping from mesh vertex attr to shader vertex attr
		ObjectPtr<ShaderResource>				mShader = nullptr;									///< The shader that this material is using
		EBlendMode								mBlendMode = EBlendMode::Opaque;					///< Blend mode for this material
		EDepthMode								mDepthMode = EDepthMode::InheritFromBlendMode;		///< Determiness how the Z buffer is used
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* UniformContainer::findUniform(const std::string& name)
	{
		UniformTextureBindings::iterator texture_binding = mUniformTextureBindings.find(name);
		if (texture_binding != mUniformTextureBindings.end() && texture_binding->second.mUniform->get_type() == RTTI_OF(T))
			return (T*)texture_binding->second.mUniform.get();

		UniformValueBindings::iterator value_binding = mUniformValueBindings.find(name);
		if (value_binding != mUniformValueBindings.end() && value_binding->second.mUniform->get_type() == RTTI_OF(T))
			return (T*)value_binding->second.mUniform.get();

		return nullptr;
	}


	template<typename T>
	T& UniformContainer::getUniform(const std::string& name)
	{
		T* result = findUniform<T>(name);
		assert(result);
		return *result;
	}


	template<typename T>
	T& MaterialInstance::getOrCreateUniform(const std::string& name)
	{
		T* existing = findUniform<T>(name);
		if (existing != nullptr)
			return *existing;
		
		return static_cast<T&>(createUniform(name));
	}
}