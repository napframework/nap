// Local Includes
#include "renderablemeshcomponent.h"
#include "mesh.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"

// External Includes
#include <nap/entity.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::Rect)
	RTTI_PROPERTY("X",		&nap::Rect::mX,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Y",		&nap::Rect::mY,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Width",	&nap::Rect::mWidth,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::Rect::mHeight,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RenderableMeshComponent)
	RTTI_PROPERTY("Mesh",				&nap::RenderableMeshComponent::mMeshResource,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableMeshComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClipRect",			&nap::RenderableMeshComponent::mClipRect,					nap::rtti::EPropertyMetaData::Default)
	RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_FUNCTION("getMaterialInstance", &nap::RenderableMeshComponentInstance::getMaterialInstance)
RTTI_END_CLASS

namespace nap
{
	// Upload all uniform variables to GPU
	void RenderableMeshComponentInstance::pushUniforms()
	{
		Material* comp_mat = mMaterialInstance.getMaterial();

		// Keep track of which uniforms were set (i.e. overridden) by the material instance
		std::unordered_set<std::string> instance_bindings;
		int texture_unit = 0;

		// Push all uniforms that are set (i.e. overridden) in the instance
		const UniformTextureBindings& instance_texture_bindings = mMaterialInstance.getTextureBindings();
		for (auto& kvp : instance_texture_bindings)
		{
			nap::Uniform* uniform_tex = kvp.second.mUniform.get();
			assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
			static_cast<nap::UniformTexture*>(uniform_tex)->push(*kvp.second.mDeclaration, texture_unit++);
			instance_bindings.insert(kvp.first);
		}				

		const UniformValueBindings& instance_value_bindings = mMaterialInstance.getValueBindings();
		for (auto& kvp : instance_value_bindings)
		{
			nap::Uniform* uniform_tex = kvp.second.mUniform.get();
			assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformValue)));
			static_cast<nap::UniformValue*>(uniform_tex)->push(*kvp.second.mDeclaration);
			instance_bindings.insert(kvp.first);
		}

		// Push all uniforms in the material that weren't overridden by the instance
		// Note that the material contains mappings for all the possible uniforms in the shader
		for (auto& kvp : comp_mat->getTextureBindings())
		{
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
			{
				nap::Uniform* uniform_val = kvp.second.mUniform.get();
				assert(uniform_val->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
				static_cast<nap::UniformTexture*>(uniform_val)->push(*kvp.second.mDeclaration, texture_unit++);
			}

		}
		for (auto& kvp : comp_mat->getValueBindings())
		{
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
			{
				nap::Uniform* uniform_val = kvp.second.mUniform.get();
				assert(uniform_val->get_type().is_derived_from(RTTI_OF(nap::UniformValue)));
				static_cast<nap::UniformValue*>(uniform_val)->push(*kvp.second.mDeclaration);
			}
		}

		glActiveTexture(GL_TEXTURE0);
	}


	void RenderableMeshComponentInstance::setBlendMode()
	{
		EDepthMode depth_mode = mMaterialInstance.getDepthMode();
		
		glDepthFunc(GL_LEQUAL);
		glBlendEquation(GL_FUNC_ADD);

		switch (mMaterialInstance.getBlendMode())
		{
		case EBlendMode::Opaque:
			glDisable(GL_BLEND);
			if (depth_mode == EDepthMode::InheritFromBlendMode)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
			}
			break;
		case EBlendMode::AlphaBlend:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if (depth_mode == EDepthMode::InheritFromBlendMode)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
			}
			break;
		case EBlendMode::Additive:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			if (depth_mode == EDepthMode::InheritFromBlendMode)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
			}
			break;
		}

		if (depth_mode != EDepthMode::InheritFromBlendMode)
		{
			switch (depth_mode)
			{
			case EDepthMode::ReadWrite:
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
				break;
			case EDepthMode::ReadOnly:
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				break;
			case EDepthMode::WriteOnly:
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
				break;
			case EDepthMode::NoReadWrite:
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				break;
			default:
				assert(false);
			}
		}
	}


	RenderableMeshComponentInstance::RenderableMeshComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource)
	{
	}


	bool RenderableMeshComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		RenderableMeshComponent* resource = getComponent<RenderableMeshComponent>();

		if (!mMaterialInstance.init(resource->mMaterialInstanceResource, errorState))
			return false;

		// Here we acquire a VAO from the render service. The service will try to reuse VAOs for similar Material-Mesh combinations
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		mVAOHandle = render_service->acquireVertexArrayObject(*mMaterialInstance.getMaterial(), *resource->mMeshResource, errorState);
		if (!errorState.check(mVAOHandle != nullptr, "Failed to acquire VAO for RenderableMeshComponent %s", resource->mID.c_str()))
			return false;

		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
 		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
 			return false;

		// Copy cliprect. Any modifications are done per instance
		mClipRect = resource->mClipRect;

		return true;
	}


	// Draw Mesh
	void RenderableMeshComponentInstance::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		if (!mVisible)
			return;

		const glm::mat4x4& model_matrix = mTransformComponent->getGlobalTransform();

		Material* comp_mat = mMaterialInstance.getMaterial();

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

		setBlendMode();
		pushUniforms();

		mVAOHandle->mObject->bind();

		// If a cliprect was set, enable scissor and set correct values
		if (mClipRect.mWidth > 0.0f && mClipRect.mHeight > 0.0f)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(mClipRect.mX, mClipRect.mY, mClipRect.mWidth, mClipRect.mHeight);
		}

		MeshInstance& mesh_instance = getMeshInstance();

		// Gather draw info
		const opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();
		GLenum draw_mode = getGLMode(mesh_instance.getDrawMode());
		const opengl::IndexBuffer* index_buffer = mesh.getIndexBuffer();

		// Draw with or without using indices
		if (index_buffer == nullptr)
		{
			glDrawArrays(draw_mode, 0, mesh_instance.getNumVertices());
		}
		else
		{
			GLsizei num_indices = static_cast<GLsizei>(index_buffer->getCount());

			index_buffer->bind();
			glDrawElements(draw_mode, num_indices, index_buffer->getType(), 0);
			index_buffer->unbind();
		}
		comp_mat->unbind();

		mVAOHandle->mObject->unbind();

		glDisable(GL_SCISSOR_TEST);
	}


	MaterialInstance& RenderableMeshComponentInstance::getMaterialInstance()
	{
		return mMaterialInstance;
	}
} 
