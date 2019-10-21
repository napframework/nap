// Local Includes
#include "renderglobals.h"

namespace nap
{
	// Projection matrix GLSL name
	const std::string projectionMatrixUniform("nap.projectionMatrix");
	
	// View matrix GLSL name
	const std::string viewMatrixUniform("nap.viewMatrix");

	// Model matrix GLSL name
	const std::string modelMatrixUniform("nap.modelMatrix");

	// GLM identity matrix
	const glm::mat4x4 identityMatrix = glm::mat4x4();
}