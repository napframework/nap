#include "RenderableMeshComponent.h"
#include "meshresource.h"
#include "RenderableMeshResource.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"

namespace nap
{
	// Draw Mesh
	void RenderableMeshComponent::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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

		Material* comp_mat = mRenderableMeshResource->getMaterial();

		comp_mat->bind();

		// Set uniform variables
		comp_mat->getUniform<UniformMat4>(projectionMatrixUniform).setValue(projectionMatrix);
		comp_mat->getUniform<UniformMat4>(viewMatrixUniform).setValue(viewMatrix);
		comp_mat->getUniform<UniformMat4>(modelMatrixUniform).setValue(model_matrix);

		comp_mat->pushUniforms();

		mRenderableMeshResource->getVAO().bind();

		// Gather draw info
		const opengl::Mesh& mesh = mRenderableMeshResource->getMeshResource()->getMesh();
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

		mRenderableMeshResource->getVAO().unbind();

	}

	RenderableMeshResource* RenderableMeshComponent::getRenderableMeshResource()
	{
		return mRenderableMeshResource.get();
	}
}

RTTI_DEFINE(nap::RenderableMeshComponent)