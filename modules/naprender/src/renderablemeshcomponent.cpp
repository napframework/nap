#include "RenderableMeshComponent.h"
#include "meshresource.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"

namespace nap
{
	// Upload all uniform variables to GPU
	void RenderableMeshComponent::pushUniforms()
	{
		Material* comp_mat = mMaterialInstance->getMaterial();

		// Keep track of which uniforms were set (i.e. overridden) by the material instance
		std::unordered_set<std::string> instance_bindings;
		int texture_unit = 0;

		// Push all uniforms that are set (i.e. overridden) in the instance
		if (mMaterialInstance != nullptr)
		{
			const UniformTextureBindings& instance_texture_bindings = mMaterialInstance->getUniformTextureBindings();
			for (auto& kvp : instance_texture_bindings)
			{
				kvp.second.mUniform->push(*kvp.second.mDeclaration, texture_unit++);
				instance_bindings.insert(kvp.first);
			}				

			const UniformValueBindings& instance_value_bindings = mMaterialInstance->getUniformValueBindings();
			for (auto& kvp : instance_value_bindings)
			{
				kvp.second.mUniform->push(*kvp.second.mDeclaration);
				instance_bindings.insert(kvp.first);
			}
		}

		// Push all uniforms in the material that weren't overridden by the instance
		// Note that the material contains mappings for all the possible uniforms in the shader
		for (auto& kvp : comp_mat->getUniformTextureBindings())
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
				kvp.second.mUniform->push(*kvp.second.mDeclaration, texture_unit++);

		for (auto& kvp : comp_mat->getUniformValueBindings())
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
				kvp.second.mUniform->push(*kvp.second.mDeclaration);

		glActiveTexture(GL_TEXTURE0);
	}


	bool RenderableMeshComponent::init(utility::ErrorState& errorState)
	{
		assert(mService->get_type() == RTTI_OF(RenderService));
		RenderService& render_service = *static_cast<RenderService*>(mService);

		mVAOHandle = render_service.acquireVertexArrayObject(*mMaterialInstance->getMaterial(), *mMeshResource, errorState);
		if (mVAOHandle == nullptr)
			return false;

		return true;
	}


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

		Material* comp_mat = mMaterialInstance->getMaterial();

		comp_mat->bind();

		// Set uniform variables
		UniformMat4* projectionUniform = comp_mat->findUniform<UniformMat4>(projectionMatrixUniform);
		if (projectionUniform != nullptr)
			projectionUniform->setValue(projectionMatrix);

		UniformMat4* viewUniform = comp_mat->findUniform<UniformMat4>(viewMatrixUniform);
		if (viewUniform != nullptr)
			viewUniform->setValue(viewMatrix);

		UniformMat4* modelUniform = comp_mat->findUniform<UniformMat4>(modelMatrixUniform);
		if (modelUniform != nullptr)
			modelUniform->setValue(model_matrix);

		pushUniforms();

		mVAOHandle->mObject->bind();

		// Gather draw info
		const opengl::Mesh& mesh = mMeshResource->getMesh();
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

		mVAOHandle->mObject->unbind();
	}


	MaterialInstance* RenderableMeshComponent::getMaterialInstance()
	{
		return mMaterialInstance.get();
	}
}

RTTI_DEFINE(nap::RenderableMeshComponent)