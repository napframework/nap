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
	class Material : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		// Default constructor
		Material();

		/**
		* Binds the GLSL shader resource program
		* Note that this call will fail when the shader is not initialized properly
		* @return if the shader was bound successfully
		*/
		bool bind();

		/**
		* Unbinds the GLSL shader resource program
		* @return if the shader was unbound successfully
		*/
		bool unbind();

		/**
		 * Utility for getting the shader resource
		 * @return the link as a shader resource, nullptr if not linked
		 */
		ShaderResource* getShader() const				{ return mShader; }

		/**
		 * Utility for checking if this material has a resolved resource
		 * @return if the shader link has been resolved and a shader resource is set
		 */
		bool  hasShader() const							{ return mShader != nullptr; }

		/**
		* Link to the shader this material uses
		* By default this link is empty, needs to be set
		* when using this material for drawing
		*/
		ResourceLinkAttribute shaderResourceLink =		{ this, "shader", RTTI_OF(ShaderResource) };

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
		 * Update shader uniforms and attribute values associated with this material
		 * Note that this function can only be called when the shader
		 * has been loaded correctly
		 */
		void resolve();

		/**
		 * Clear shader bindings
		 */
		void clear();

		/**
		 * Occurs when the resource changes, ie: link is removed or new
		 * object is linked in. In that case we want to listen to when the 
		 * shader is loaded or reloaded
		 */
		void onResourceLinkChanged(AttributeBase& value);

		/**
		 * Occurs when the shader this material points to
		 * is loaded or reloaded, based on the success we 
		 * update our uniforms
		 */
		void onShaderLoaded(bool success);

		/**
		 * Internal resource cache
		 */
		ShaderResource* mShader = nullptr;

		/**
		 * Tries to resolve the link and cache the shader
		 */
		void resolveShaderLink();

		NSLOT(shaderResourceChanged, AttributeBase&, onResourceLinkChanged)
		NSLOT(shaderLoaded, bool, onShaderLoaded)
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
