// local includes
#include "transformcomponent.h"

// External includes
#include <glm/gtx/transform.hpp>

namespace nap
{
	// Constructs and returns this components local transform
	glm::mat4x4 TransformComponent::getLocalTransform() const
	{
		glm::mat4x4 model_matrix;
		model_matrix = glm::translate(model_matrix, translate.getValue());
		model_matrix = glm::rotate(model_matrix, glm::radians(rotate.getValue().x), glm::vec3(1.0, 0.0, 0.0));
		model_matrix = glm::rotate(model_matrix, glm::radians(rotate.getValue().y), glm::vec3(0.0, 1.0, 0.0));
		model_matrix = glm::rotate(model_matrix, glm::radians(rotate.getValue().z), glm::vec3(0.0, 0.0, 1.0));
		model_matrix = glm::scale(model_matrix, scale.getValue() * uniformScale.getValue());
		return model_matrix;
	}
}
RTTI_DEFINE(nap::TransformComponent)