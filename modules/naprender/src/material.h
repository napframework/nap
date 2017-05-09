#pragma once

// External includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/resourcelinkattribute.h>

// Local includes
#include "shaderresource.h"
#include "imageresource.h"
#include "nmesh.h"

namespace nap
{
	/**
	* Instance of a shader that lives in your object tree
	* Holds a pointer to the actual shader resource program
	* Can be used to draw an object to screen
	* Typically a material instance is a child of a mesh component
	*/
	class Material : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		struct VertexAttributeBinding
		{
			std::string mMeshAttributeID;
			std::string mShaderAttributeID;
		};

		// Default constructor
		Material();

		/**
		* Creates mappings for uniform and vertex attrs.
		*/
		virtual bool init(ErrorState& errorState) override;

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
		 * Template uniform set function
		 * @param name: name of the uniform variable
		 * @param value: value to set
		 */
		template<typename T>
		void setUniformValue(const std::string& name, const T& value);

		/**
		 * Uniform texture set function
		 * @param name: name of the texture binding
		 * @param resource: resource to link the uniform binding to, TODO: Should be const (Coen)!
		 */
		void setUniformTexture(const std::string& name, TextureResource& resource);
		
		const VertexAttributeBinding* findVertexAttributeBinding(const opengl::VertexAttributeID& shaderAttributeID) const;

	public:
		std::vector<VertexAttributeBinding> mVertexAttributeBindings;

	private:

		/**
		* Holds all uniform shader variables
		*/
		CompoundAttribute uniformAttribute = { this, "uniforms" };

	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	void nap::Material::setUniformValue(const std::string& name, const T& value)
	{
		Attribute<T>* attr = uniformAttribute.getAttribute<T>(name);
		if (attr == nullptr)
		{
			nap::Logger::warn(*this, "uniform variable: %s does not exist", name.c_str());
			return;
		}
		attr->setValue(value);
	}

}
