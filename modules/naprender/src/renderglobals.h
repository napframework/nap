#pragma once

// External Includes
#include <string>
#include <glm/glm.hpp>

#include <utility/dllexport.h>

namespace nap
{
	/**
	 * GLSL uniform attribute name for root nap struct
	 */
	constexpr const char* mvpStructUniform = "nap";

	/**
	 * GLSL uniform attribute name of model matrix
	 */
	constexpr const char* modelMatrixUniform = "modelMatrix";

	/**
	 * GLSL uniform attribute name for view matrix
	 */
	constexpr const char* viewMatrixUniform = "viewMatrix";

	/**
	 * GLSL uniform attribute name for projection matrix
	 */
	constexpr const char* projectionMatrixUniform = "projectionMatrix";
}
