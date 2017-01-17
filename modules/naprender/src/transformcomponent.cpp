// local includes
#include "transformcomponent.h"

// External includes
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace nap
{
	// Constructs and returns this components local transform
	glm::mat4x4 TransformComponent::getLocalTransform() const
	{
		glm::mat4x4 xform_matrix = glm::translate(glm::mat4x4(), translate.getValue());
		glm::mat4x4 rotat_matrix = glm::toMat4(glm::quat(rotate.getValue()));
		glm::mat4x4 scale_matrix = glm::scale(scale.getValue() * uniformScale.getValue());
		return xform_matrix * rotat_matrix * scale_matrix;
	}
}
RTTI_DEFINE(nap::TransformComponent)