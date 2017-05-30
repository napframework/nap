#include "RenderableMeshComponent.h"
#include "meshresource.h"
#include "RenderableMeshResource.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"

namespace nap
{
	// Upload all uniform variables to GPU
	void RenderableMeshComponent::pushUniforms()
	{
		Material* comp_mat = mRenderableMeshResource->getMaterial();

		// Build a list of all the uniforms that we are going to set
		std::unordered_map<std::string, const UniformBinding<UniformTexture>*> texture_bindings;
		const UniformTextureBindings& shared_texture_bindings = comp_mat->getUniformTextureBindings();
		texture_bindings.reserve(shared_texture_bindings.size());

		std::unordered_map<std::string, const UniformBinding<UniformValue>*> value_bindings;
		const UniformValueBindings& shared_value_bindings = comp_mat->getUniformValueBindings();
		value_bindings.reserve(value_bindings.size());

		// Add all the uniforms present in the instance
		if (mMaterialInstance != nullptr)
		{
			const UniformTextureBindings& instance_texture_bindings = mMaterialInstance->getUniformTextureBindings();
			for (auto& kvp : instance_texture_bindings)
				texture_bindings.emplace(std::make_pair(kvp.first, &kvp.second));

			const UniformValueBindings& instance_value_bindings = mMaterialInstance->getUniformValueBindings();
			for (auto& kvp : instance_value_bindings)
				value_bindings.emplace(std::make_pair(kvp.first, &kvp.second));
		}

		// Add all the uniforms from the material that weren't set yet
		// Note that the material contains mappings for all the possible uniforms in the shader
		for (auto& kvp : shared_texture_bindings)
			if (texture_bindings.find(kvp.first) == texture_bindings.end())
				texture_bindings.emplace(std::make_pair(kvp.first, &kvp.second));

		for (auto& kvp : shared_value_bindings)
			if (value_bindings.find(kvp.first) == value_bindings.end())
				value_bindings.emplace(std::make_pair(kvp.first, &kvp.second));

		// Push values
		for (auto& kvp : value_bindings)
			kvp.second->mUniform->push(*kvp.second->mDeclaration);

		// Push textures
		int texture_unit = 0;
		for (auto& kvp : texture_bindings)
			kvp.second->mUniform->push(*kvp.second->mDeclaration, texture_unit++);

		glActiveTexture(GL_TEXTURE0);
	}


	bool RenderableMeshComponent::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mMaterialInstance == nullptr || mMaterialInstance->getMaterial() == mRenderableMeshResource->getMaterial(), "MaterialInstance does not override the material from the RenderableMeshResource"))
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

		Material* comp_mat = mRenderableMeshResource->getMaterial();

		comp_mat->bind();

		// Set uniform variables
		comp_mat->getUniform<UniformMat4>(projectionMatrixUniform).setValue(projectionMatrix);
		comp_mat->getUniform<UniformMat4>(viewMatrixUniform).setValue(viewMatrix);
		comp_mat->getUniform<UniformMat4>(modelMatrixUniform).setValue(model_matrix);

		pushUniforms();

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

	MaterialInstance* RenderableMeshComponent::getMaterialInstance()
	{
		return mMaterialInstance.get();
	}
}

RTTI_DEFINE(nap::RenderableMeshComponent)