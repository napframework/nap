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
	* GLSL set function, where is the data, GLint the location and
	* GLSizei the number of elements
	*/
	using GLSLSetterFunction = std::function<void(const opengl::UniformVariable&, const AttributeBase&)>;

	/**
	 * GLSL attribute create function, where compound attribute is the attribute parent
	 */
	using GLSLAttributeCreateFunction = std::function<AttributeBase&(const opengl::UniformVariable& uvar, CompoundAttribute& compound)>;

	/**
	* @return the attribute type associated with a certain GLSL shader input type
	* returns invalid if the attribute type is not supported
	*/
	RTTI::TypeInfo getAttributeType(opengl::GLSLType type);

	/**
	* @return an attribute create function based on a certain GLSL type
	* nullptr if GLSL type is not supported
	*/
	const GLSLAttributeCreateFunction* getAttributeCreateFunction(opengl::GLSLType type);

	/**
	 * @return an attribute set function based on a certain rtti type
	 * nullptr if RTTI type is not supported
	 */
	const GLSLSetterFunction* getGLSLSetFunction(const RTTI::TypeInfo& type);
}
