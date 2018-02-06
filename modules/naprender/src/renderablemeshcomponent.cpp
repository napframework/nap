// Local Includes
#include "renderablemeshcomponent.h"
#include "mesh.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::RenderableMeshComponent)
	RTTI_PROPERTY("Mesh",				&nap::RenderableMeshComponent::mMesh,						nap::rtti::EPropertyMetaData::Default)
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
		MaterialInstance& material_instance = getMaterialInstance();
		Material* material = material_instance.getMaterial();

		// Keep track of which uniforms were set (i.e. overridden) by the material instance
		std::unordered_set<std::string> instance_bindings;
		int texture_unit = 0;

		// Push all uniforms that are set (i.e. overridden) in the instance
		const UniformTextureBindings& instance_texture_bindings = material_instance.getTextureBindings();
		for (auto& kvp : instance_texture_bindings)
		{
			nap::Uniform* uniform_tex = kvp.second.mUniform.get();
			assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
			static_cast<nap::UniformTexture*>(uniform_tex)->push(*kvp.second.mDeclaration, texture_unit++);
			instance_bindings.insert(kvp.first);
		}				

		const UniformValueBindings& instance_value_bindings = material_instance.getValueBindings();
		for (auto& kvp : instance_value_bindings)
		{
			nap::Uniform* uniform_tex = kvp.second.mUniform.get();
			assert(uniform_tex->get_type().is_derived_from(RTTI_OF(nap::UniformValue)));
			static_cast<nap::UniformValue*>(uniform_tex)->push(*kvp.second.mDeclaration);
			instance_bindings.insert(kvp.first);
		}

		// Push all uniforms in the material that weren't overridden by the instance
		// Note that the material contains mappings for all the possible uniforms in the shader
		for (auto& kvp : material->getTextureBindings())
		{
			if (instance_bindings.find(kvp.first) == instance_bindings.end())
			{
				nap::Uniform* uniform_val = kvp.second.mUniform.get();
				assert(uniform_val->get_type().is_derived_from(RTTI_OF(nap::UniformTexture)));
				static_cast<nap::UniformTexture*>(uniform_val)->push(*kvp.second.mDeclaration, texture_unit++);
			}

		}
		for (auto& kvp : material->getValueBindings())
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
		EDepthMode depth_mode = getMaterialInstance().getDepthMode();
		
		glDepthFunc(GL_LEQUAL);
		glBlendEquation(GL_FUNC_ADD);

		switch (getMaterialInstance().getBlendMode())
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


	bool RenderableMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		RenderableMeshComponent* resource = getComponent<RenderableMeshComponent>();

		if (!mMaterialInstance.init(resource->mMaterialInstanceResource, errorState))
			return false;

		// A mesh isn't required, it may be set by a derived class or by some other code through setMesh
		// If it is set, we create a renderablemesh from it
		if (resource->mMesh != nullptr)
		{
			mRenderableMesh = createRenderableMesh(*resource->mMesh, mMaterialInstance, errorState);
			if (!errorState.check(mRenderableMesh.isValid(), "Unable to create renderable mesh"))
				return false;
		}

		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
 		if (!errorState.check(mTransformComponent != nullptr, "Missing transform component"))
 			return false;

		// Copy cliprect. Any modifications are done per instance
		mClipRect = resource->mClipRect;

		return true;
	}


	RenderableMesh RenderableMeshComponentInstance::createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		VAOHandle vao_handle = render_service->acquireVertexArrayObject(*materialInstance.getMaterial(), mesh, errorState);

		if (!errorState.check(vao_handle.isValid(), "Failed to acquire VAO for RenderableMeshComponent %s", getComponent()->mID.c_str()))
			return RenderableMesh();

		return RenderableMesh(mesh, materialInstance, vao_handle);
	}


	RenderableMesh RenderableMeshComponentInstance::createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState)
	{
		return createRenderableMesh(mesh, mMaterialInstance, errorState);
	}


	void RenderableMeshComponentInstance::setMesh(const RenderableMesh& mesh)
	{
		assert(mesh.isValid());
		mRenderableMesh = mesh;
	}


	// Draw Mesh
	void RenderableMeshComponentInstance::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		if (!mVisible || !mRenderableMesh.isValid())
			return;

		const glm::mat4x4& model_matrix = mTransformComponent->getGlobalTransform();

		Material* comp_mat = getMaterialInstance().getMaterial();

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

		mRenderableMesh.mVAOHandle.get().bind();

		// If a cliprect was set, enable scissor and set correct values
		if (mClipRect.hasWidth() && mClipRect.hasHeight())
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(mClipRect.getMin().x, mClipRect.getMin().y, mClipRect.getWidth(), mClipRect.getHeight());
		}

		MeshInstance& mesh_instance = getMeshInstance();

		// Gather draw info
		const opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();

		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			MeshShape& shape = mesh_instance.getShape(index);
			const opengl::IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			
			GLenum draw_mode = getGLMode(shape.getDrawMode());
			GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());

			index_buffer.bind();
			glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
			index_buffer.unbind();
		}

		comp_mat->unbind();

		mRenderableMesh.mVAOHandle.get().unbind();

		glDisable(GL_SCISSOR_TEST);
	}


	MaterialInstance& RenderableMeshComponentInstance::getMaterialInstance()
	{
		return *mRenderableMesh.mMaterialInstance;
	}
} 
