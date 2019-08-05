#pragma once

// External includes
#include <nap/resourceptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>

// Local includes
#include "shader.h"
#include "imagefromfile.h"
#include "ngpumesh.h"
#include "uniformbinding.h"

namespace nap
{
	class Material;
	class MaterialInstance;

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
		std::vector<ResourcePtr<Uniform>>	mUniforms;										///< Property: "Uniforms" that you're overriding
		ResourcePtr<Material>				mMaterial;										///< Property: "Material" that you're overriding uniforms from
		EBlendMode							mBlendMode = EBlendMode::NotSet;				///< Property: "BlendMode" Blend mode override. By default uses material blend mode
		EDepthMode							mDepthMode = EDepthMode::NotSet;				///< Property: "DepthMode" Depth mode override. By default uses material depth mode
	};

	/**
	 * MaterialInstance holds overloads of Material properties. It is intended to be used as a field in 
	 * ComponentInstances. It needs to be initialized with a MaterialInstanceResource object to fill it's
	 * runtime data. init() needs to be called from the ComponentInstance's init() function.
	 */
	class NAPAPI MaterialInstance : public UniformContainer
	{
		RTTI_ENABLE(UniformContainer)
	public:
		/**
		 * For each uniform in mUniforms, creates a mapping.
		 */
		bool init(MaterialInstanceResource& resource, utility::ErrorState& errorState);

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
		* getMaterial().getUniforms().getUniform().
		* This function will assert if the name of the uniform does not match the type that you are trying to
		* create.
		* @param name: the name of the uniform as it is in the shader.
		* @return reference to the uniform that was found or created.
		*/
		Uniform* getOrCreateUniform(const std::string& name);

		/**
		* Get a uniform for this material instance. This means that the uniform returned is only applicable
		* to this instance. In order to change a uniform so that it's value is shared among materials, use
		* getMaterial().getUniforms().getUniform().
		* This function will assert if the name of the uniform does not match the type that you are trying to
		* create.
		* @param name: the name of the uniform as it is in the shader.
		* @return reference to the uniform that was found or created.
		*/
		template<typename T>
		T& getOrCreateUniform(const std::string& name);

		/**
		 * Binds the shader program so it can be used by subsequent calls such as pushUniforms() etc.
		 * This call is forwarded to bind() of the parent material.
		 */
		void bind();

		/**
		 * Unbinds the shader program.
		 * This call is forwarded to unbind() of the parent material.
		 */
		void unbind();

		/**
		 * Uploads all uniforms stored in this material to the GPU. Call this after binding!
		 * Only call this after binding the material otherwise the outcome of this call is uncertain.
		 * This applies to the uniforms in the instance that are overridden as for the uniforms in the underlying material.
		 */
		void pushUniforms();

		/**
		 * Updates the blend mode on the GPU based on the blend settings associated with this material.
		 * Both the instance and parent material have the option to change the blend mode.
		 * If this material inherits the blend mode from the parent material the blend mode of the parent material is used.
		 * Otherwise the blend settings that have been given to this instance are taken into account. 
		 * Preferably call this after binding!
		 */
		void pushBlendMode();

		/**
		 * Locates the texture unit that is associated with a specific uniform in this material.
		 * Use this index when updating a specific texture uniform on the GPU.
		 * The texture unit number is required when pushing a single texture to the GPU.
		 * Note that the uniform needs to be managed (created) by this material instance or the underlying parent material.
		 * @param uniform the texture uniform to find the texture unit number for.
		 * @return the texture unit associated with a specific texture uniform in a material, -1 if not found
		 */
		int getTextureUnit(nap::UniformTexture& uniform);

	private:
		/**
		 * Creates a new uniform with the given name
		 */
		Uniform& createUniform(const std::string& name);

		//. Resource this instance is associated with
		MaterialInstanceResource* mResource;
	};


	/**
	* Material can be considered as the interface towards a shader.
	* It contains default mappings for how mesh vertex attributes are bound to a shader's vertex attributes.
	* It also holds the uniform values for all the uniforms present in the shader. If a uniform is updated 
	* on the material, all the objects that use this material will use that value. To change uniform values
	* per object, set uniform values on MaterialInstances.
	*/
	class NAPAPI Material : public Resource, public UniformContainer
	{
		RTTI_ENABLE(Resource)
	public:

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

		// Default constructor
		Material() = default;

		/**
		* Creates mappings for uniform and vertex attrs.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

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

	private:
		/**
		 * Recursively add uniforms for the specified declaration
		 */
		Uniform* addUniformRecursive(const opengl::UniformDeclaration& declaration, const std::string& path, const std::vector<std::string>& parts, int partIndex, bool& didCreateUniform);

		/**
		 * Ensure a UniformStruct with the specified name is created. The globalName is an identifier that should be globally unique; the localName is an
		 * identifier that should only be unique within the container that the uniform is being added to
		 */
		UniformStruct& getOrCreateUniformStruct(const std::string& globalName, const std::string& localName, bool& created);

		/**
		 * Ensure a UniformStructArray with the specified name is created. The globalName is an identifier that should be globally unique; the localName is an
		 * identifier that should only be unique within the container that the uniform is being added to
		 */
		UniformStructArray& getOrCreateUniformStructArray(const std::string& globalName, const std::string& localName, bool& created);

	public:
		std::vector<ResourcePtr<Uniform>>		mUniforms;											///< Property: 'Uniforms' Static uniforms (as read from file, or as set in code before calling init())
		std::vector<VertexAttributeBinding>		mVertexAttributeBindings;							///< Property: 'VertexAttributeBindings' Optional, mapping from mesh vertex attr to shader vertex attr
		ResourcePtr<Shader>						mShader = nullptr;									///< Property: 'Shader' The shader that this material is using
		EBlendMode								mBlendMode = EBlendMode::Opaque;					///< Property: 'BlendMode' Optional, blend mode for this material
		EDepthMode								mDepthMode = EDepthMode::InheritFromBlendMode;		///< Property: 'DepthMode' Optional, determines how the Z buffer is used

	private:
		using UniformStructMap = std::unordered_map<std::string, std::unique_ptr<UniformStruct>>;
		using UniformStructArrayMap = std::unordered_map<std::string, std::unique_ptr<UniformStructArray>>;

		UniformStructMap						mOwnedStructUniforms;								///< Runtime map of struct uniforms
		UniformStructArrayMap					mOwnedStructArrayUniforms;							///< Runtime map of struct array uniforms
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T& MaterialInstance::getOrCreateUniform(const std::string& name)
	{
		// Find the uniform based on name
		T* existing = findUniform<T>(name);
		if (existing != nullptr)
			return *existing;
		
		// Create the uniform if it can't be found
		Uniform& new_uniform = createUniform(name);

		// If the cast fails it means the requested uniform types do not match!
		T* cast_uniform = rtti_cast<T>(&new_uniform);
		assert(cast_uniform != nullptr);
		return *cast_uniform;
	}
}
