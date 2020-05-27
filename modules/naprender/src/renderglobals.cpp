// Local Includes
#include "renderglobals.h"

namespace nap
{
	const std::string mvpStructUniform("nap");

	// Projection matrix GLSL name
	const std::string projectionMatrixUniform("projectionMatrix");
	
	// View matrix GLSL name
	const std::string viewMatrixUniform("viewMatrix");

	// Model matrix GLSL name
	const std::string modelMatrixUniform("modelMatrix");

	// GLM identity matrix
	const glm::mat4x4 identityMatrix = glm::mat4x4();
}