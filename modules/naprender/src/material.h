#pragma once

// External includes
#include <nap/serviceablecomponent.h>
#include <nap/attribute.h>
#include <nap/resourcelinkattribute.h>

// Local includes
#include "shaderresource.h"

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
		Material() = default;

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
		* @return if there's a shader resource linked to this instance
		*/
		bool hasShader() const { return shaderResource.isLinked(); }

		/**
		* Link to the shader this material uses
		* By default this link is empty, needs to be set
		* when using this material for drawing
		*/
		ResourceLinkAttribute shaderResource = { this, "shader", RTTI_OF(ShaderResource) };

		/**
		 * Utility for getting the shader resource
		 * @return the link as a shader resource, nullptr if not linked
		 */
		ShaderResource* getResource();
	};
}

RTTI_DECLARE(nap::Material)
