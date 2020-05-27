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
	constexpr char* mvpStructUniform = "nap";

	/**
	 * GLSL uniform attribute name of model matrix
	 */
	constexpr char* modelMatrixUniform = "modelMatrix";

	/**
	 * GLSL uniform attribute name for view matrix
	 */
	constexpr char* viewMatrixUniform = "viewMatrix";

	/**
	 * GLSL uniform attribute name for projection matrix
	 */
	constexpr char* projectionMatrixUniform = "projectionMatrix";
}
