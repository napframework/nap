#pragma once

// External includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/resourcelinkattribute.h>

// Local includes
#include "shaderresource.h"
#include "imageresource.h"
#include "uniforms.h"
#include "nmesh.h"

namespace nap
{
	/**
	* Material can be considered as the interface towards a shader. 
	* It contains default mappings for how mesh vertex attributes are bound to a shader's vertex attributes.
	* It also holds the actual uniform values that are pushed into the shader at runtime.
	*/
	class Material : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		* Binding betwee mesh vertex attr and shader vertex attr
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
		* Performs commit or rollback of changes made in init()
		*/
		virtual void finish(Resource::EFinishMode mode) override;

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
		ShaderResource* getShader() const				{ return mShader; }

		/**
		* Link to the shader this material uses
		* By default this link is empty, needs to be set
		* when using this material for drawing
		*/
		ShaderResource* mShader = nullptr;

		/**
		 * Uploads all uniform variables to the GPU
		 * Note that this call will only work when the shader is bound!
		 */
		void pushUniforms();

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
		* Finds the mesh/shader attribute binding based on the shader attribute ID.
		* @param shaderAttributeID: ID of the shader vertex attribute.
		*/
		const VertexAttributeBinding* findVertexAttributeBinding(const opengl::Mesh::VertexAttributeID& shaderAttributeID) const;

		/**
		* @return Returns a mapping with default values for mesh attribute IDs an shader attribute IDs.
		*/
		static std::vector<VertexAttributeBinding>& getDefaultVertexAttributeBindings();

	public:
		std::vector<VertexAttributeBinding> mVertexAttributeBindings;		///< Mapping from mesh vertex attr to shader vertex attr
		std::vector<Uniform*>				mUniforms;						///< Static uniforms (as read from file, or as set in code before calling init())

	private:

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
		UniformTextureBindings	mUniformTextureBindings;			///< Runtime map of texture uniforms (superset of texture uniforms in mUniforms due to default uniforms).
		UniformTextureBindings	mPrevUniformTextureBindings;		///< For commit/rollback
		UniformValueBindings	mUniformValueBindings;;				///< Runtime map of value uniforms (superset of value uniforms in mUniforms due to default uniforms).
		UniformValueBindings	mPrevUniformValueBindings;			///< For commit/rollback
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* nap::Material::findUniform(const std::string& name)
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
	T& nap::Material::getUniform(const std::string& name)
	{
		T* result = findUniform<T>(name);
		assert(result);
		return *result;
	}
}
