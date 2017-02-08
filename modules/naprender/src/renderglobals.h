#pragma once

// External Includes
#include <string>
#include <glm/glm.hpp>

namespace nap
{
	/**
	 * GLSL uniform attribute name for projection matrix
	 */
	extern const std::string projectionMatrixUniform;

	/**
	 * GLSL uniform attribute name for view matrix
	 */
	extern const std::string viewMatrixUniform;

	/**
	 * GLSL uniform attribute name of model matrix
	 */
	extern const std::string modelMatrixUniform;

	/**
	 * GLM Identity Matrix
	 */
	extern const glm::mat4x4 identityMatrix;
}
