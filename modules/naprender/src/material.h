#pragma once

// External includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/resourcelinkattribute.h>

// Local includes
#include "shaderresource.h"
#include "imageresource.h"

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
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		// Default constructor
		Material();

		virtual bool init(InitResult& initResult) override;

		virtual void finish(Resource::EFinishMode mode) override;

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
		 * Holds all uniform shader variables
		 */
		CompoundAttribute uniformAttribute =			{ this, "uniforms" };

		/**
		 * Holds all vertex attribute variables
		 */
		CompoundAttribute vertexAttribute =				{ this, "attributes" };

		/**
		 * Uploads all uniform variables to the GPU
		 * Note that this call will only work when the shader is bound!
		 */
		void pushUniforms();

		/**
		 * Updates the variable attribute bindings on the GPU
		 * Note that this call needs to be called before binding the material
		 * Shader index operations are only pushed on a new bind because of linking impact
		 */
		void pushAttributes();

		/**
		 * Updates the vertex attribute binding location
		 * The binding location is associated with a specific buffer when drawing
		 * Every buffer has it's own location withing the array object that is used
		 * @param name: Name of the vertex attribute in the shader
		 * @param location: New buffer location to use when drawing
		 */
		void linkVertexBuffer(const std::string& name, int location);

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
		
	private:

		/**
		 * Internal resource cache
		 */
		CompoundAttribute mPrevUniformAttributes;
		CompoundAttribute mPrevVertexAttributes;
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

RTTI_DECLARE(nap::Material)
