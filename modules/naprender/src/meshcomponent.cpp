#include "meshcomponent.h"
#include "meshresource.h"
#include "modelresource.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"

namespace nap
{
	// Draw Mesh
	void MeshComponent::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get xform component
		nap::Entity* parent_entity = getParent();
		assert(parent_entity != nullptr);
		TransformComponent* xform_comp = parent_entity->getComponent<TransformComponent>();

		// Make sure it exists and extract global matrix
		if (xform_comp == nullptr)
		{
			nap::Logger::warn("render able object has no transform: %s", getName().c_str());
		}
		const glm::mat4x4& model_matrix = xform_comp == nullptr ? identityMatrix : xform_comp->getGlobalTransform();

		Material* comp_mat = mModelResource->getMaterial();

		comp_mat->bind();

		// Set uniform variables
		comp_mat->setUniformValue<glm::mat4x4>(projectionMatrixUniform, projectionMatrix);
		comp_mat->setUniformValue<glm::mat4x4>(viewMatrixUniform, viewMatrix);
		comp_mat->setUniformValue<glm::mat4x4>(modelMatrixUniform, model_matrix);

		comp_mat->pushUniforms();

		mModelResource->getVAO().bind();

		// Gather draw info
		const opengl::Mesh& mesh = mModelResource->getMeshResource()->getMesh();
		GLenum draw_mode = getGLMode(mesh.getDrawMode());
		const opengl::IndexBuffer* index_buffer = mesh.getIndexBuffer();
		GLsizei draw_count = static_cast<GLsizei>(index_buffer->getCount());

		// Draw with or without using indices
		if (index_buffer == nullptr)
		{
			glDrawArrays(draw_mode, 0, draw_count);
		}
		else
		{
			index_buffer->bind();
			glDrawElements(draw_mode, draw_count, index_buffer->getType(), 0);
			index_buffer->unbind();
		}
		comp_mat->unbind();

		mModelResource->getVAO().unbind();

	}
}

RTTI_DEFINE(nap::MeshComponent)