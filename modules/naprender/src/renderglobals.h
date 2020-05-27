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
	NAPAPI extern const std::string mvpStructUniform;

	/**
	 * GLSL uniform attribute name of model matrix
	 */
	NAPAPI extern const std::string modelMatrixUniform;

	/**
	 * GLSL uniform attribute name for view matrix
	 */
	NAPAPI extern const std::string viewMatrixUniform;

	/**
	 * GLSL uniform attribute name for projection matrix
	 */
	NAPAPI extern const std::string projectionMatrixUniform;

	/**
	 * GLM Identity Matrix
	 */
	NAPAPI extern const glm::mat4x4 identityMatrix;
}
