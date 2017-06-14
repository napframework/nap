#include "RenderableMeshComponent.h"
#include "meshresource.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"

RTTI_BEGIN_CLASS(nap::Rect)
	RTTI_PROPERTY("X",		&nap::Rect::mX,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Y",		&nap::Rect::mY,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Width",	&nap::Rect::mWidth,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::Rect::mHeight,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RenderableMeshComponentResource)
	RTTI_PROPERTY("Mesh",				&nap::RenderableMeshComponentResource::mMeshResource,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableMeshComponentResource::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClipRect",			&nap::RenderableMeshComponentResource::mClipRect,					nap::rtti::EPropertyMetaData::Default)
	RTTI_END_CLASS

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::RenderableMeshComponent, nap::EntityInstance&)
RTTI_END_CLASS

namespace nap
{
	// Upload all uniform variables to GPU
	void RenderableMeshComponent::pushUniforms()
	{
		Material* comp_mat = mMaterialInstance.getMaterial();

		// Keep track of which uniforms were set (i.e. overridden) by the material instance
		std::unordered_set<std::string> instance_bindings;
		int texture_unit = 0;

		// Push all uniforms that are set (i.e. overridden) in the instance
		const UniformTextureBindings& instance_texture_bindings = mMaterialInstance.getUniformTextureBindings();
		for (auto& kvp : instance_texture_bindings)
		{
			kvp.second.mUniform->push(*kvp.second.mDeclaration, texture_unit++);
			instance_bindings.insert(kvp.first);
		}				

		const UniformValueBindings& instance_value_bindings = mMaterialInstance.getUniformValueBindings();
		for (auto& kvp : instance_value_bindings)
		{
			kvp.second.mUniform->push(*kvp.second.mDeclaration);
			instance_bindings.insert(kvp.first);
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


	void RenderableMeshComponent::setBlendMode()
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

	RenderableMeshComponent::RenderableMeshComponent(EntityInstance& entity) :
		RenderableComponent(entity)
	{
	}

	bool RenderableMeshComponent::init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState)
	{
		assert(resource->get_type().is_derived_from<RenderableMeshComponentResource>());
		mResource = rtti_cast<RenderableMeshComponentResource>(resource.get());

		if (!mMaterialInstance.init(mResource->mMaterialInstanceResource, errorState))
			return false;

		RenderService* render_service = getEntity()->getCore()->getService<RenderService>();
		mVAOHandle = render_service->acquireVertexArrayObject(*mMaterialInstance.getMaterial(), *mResource->mMeshResource, errorState);
		if (mVAOHandle == nullptr)
			return false;

		mTransformComponent = getEntity()->findComponent<TransformComponent>();
 		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
 			return false;

		mClipRect = mResource->mClipRect;

		return true;
	}


	// Draw Mesh
	void RenderableMeshComponent::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
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

		// Gather draw info
		const opengl::Mesh& mesh = mResource->mMeshResource->getMesh();
		GLenum draw_mode = getGLMode(mesh.getDrawMode());
		const opengl::IndexBuffer* index_buffer = mesh.getIndexBuffer();
		GLsizei draw_count = static_cast<GLsizei>(index_buffer->getCount());

		if (mClipRect.mWidth > 0.0f && mClipRect.mHeight > 0.0f)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(mClipRect.mX, mClipRect.mY, mClipRect.mWidth, mClipRect.mHeight);
		}

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

		glDisable(GL_SCISSOR_TEST);
	}


	MaterialInstance& RenderableMeshComponent::getMaterialInstance()
	{
		return mMaterialInstance;
	}
}

