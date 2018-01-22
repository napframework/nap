#pragma once

// External includes
#include <nap/objectptr.h>
#include <utility/dllexport.h>

// Local includes
#include "shader.h"
#include "image.h"
#include "ngpumesh.h"
#include "uniformbinding.h"

namespace nap
{
	class Material;
	class MaterialInstance;

	/**
	* Blend mode for Materials.
	*/
	enum class NAPAPI EBlendMode
	{
		NotSet,					///< Default value for MaterialInstances, means that the Material's blend mode is used instead
		Opaque,					///< Regular opaque, similar to (One, Zero) blend
		AlphaBlend,				///< Transparant object (SrcAlpha, InvSrcAlpha) blend
		Additive				///< Additive, (One, One) blend
	};

	/**
	* Determines how to z-buffer is used for reading and writing.
	* When inheriting from blend mode
	*/
	enum class NAPAPI EDepthMode
	{
		NotSet,					///< Default value for MaterialInstances, means that the Material's blend is used instead
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
		std::vector<ObjectPtr<Uniform>>		mUniforms;										///< Property: "Uniforms" that you're overriding
		ObjectPtr<Material>					mMaterial;										///< Property: "Material" that you're overriding uniforms from
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
	class NAPAPI Material : public rtti::RTTIObject, public UniformContainer
	{
		RTTI_ENABLE(rtti::RTTIObject)
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

	public:
		std::vector<ObjectPtr<Uniform>>			mUniforms;											///< Static uniforms (as read from file, or as set in code before calling init())
		std::vector<VertexAttributeBinding>		mVertexAttributeBindings;							///< Mapping from mesh vertex attr to shader vertex attr
		ObjectPtr<Shader>						mShader = nullptr;									///< The shader that this material is using
		EBlendMode								mBlendMode = EBlendMode::Opaque;					///< Blend mode for this material
		EDepthMode								mDepthMode = EDepthMode::InheritFromBlendMode;		///< Determines how the Z buffer is used									///< Holds all the uniform values
	};

	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T& MaterialInstance::getOrCreateUniform(const std::string& name)
	{
		T* existing = findUniform<T>(name);
		if (existing != nullptr)
			return *existing;
		
		return static_cast<T&>(createUniform(name));
	}
}