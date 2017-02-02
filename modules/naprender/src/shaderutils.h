#pragma once

// External Includes
#include <nshaderutils.h>

// Local Includes
#include "renderattributes.h"

namespace nap
{
	// Forward Declares
	class ShaderResource;

	/**
	 * @return the attribute type associated with a certain GLSL shader input type
	 * returns invalid if the attribute type is not supported
	 */
	RTTI::TypeInfo getAttributeType(opengl::GLSLType type);

	/**
	* GLSL set function, where is the data, GLint the location and
	* GLSizei the number of elements
	*/
	using GLSLSetterFunction = std::function<void(const ShaderResource&, const AttributeBase&)>;
}
