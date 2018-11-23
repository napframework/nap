// Local Includes
#include "renderablemeshcomponent.h"
#include "mesh.h"
#include "ncamera.h"
#include "transformcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "renderservice.h"
#include "materialutils.h"

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

	void RenderableMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.push_back(RTTI_OF(TransformComponent));
	}


	// Upload all uniform variables to GPU
	void RenderableMeshComponentInstance::pushUniforms()
	{
		utility::pushUniforms(getMaterialInstance());
	}


	void RenderableMeshComponentInstance::setBlendMode()
	{
		utility::setBlendMode(getMaterialInstance());
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
		return render_service->createRenderableMesh(mesh, materialInstance, errorState);
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
	void RenderableMeshComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{	
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Get global transform
		const glm::mat4x4& model_matrix = mTransformComponent->getGlobalTransform();

		// Get material to bind
		Material* comp_mat = getMaterialInstance().getMaterial();
		comp_mat->bind();

		// Set projection uniform in shader
		UniformMat4* projectionUniform = comp_mat->findUniform<UniformMat4>(projectionMatrixUniform);
		if (projectionUniform != nullptr)
			projectionUniform->setValue(projectionMatrix);

		// Set view uniform in shader
		UniformMat4* viewUniform = comp_mat->findUniform<UniformMat4>(viewMatrixUniform);
		if (viewUniform != nullptr)
			viewUniform->setValue(viewMatrix);

		// Set model matrix uniform in shader
		UniformMat4* modelUniform = comp_mat->findUniform<UniformMat4>(modelMatrixUniform);
		if (modelUniform != nullptr)
			modelUniform->setValue(model_matrix);

		// Prepare blending
		setBlendMode();

		// Push all shader uniforms
		pushUniforms();

		mRenderableMesh.bind();

		// If a cliprect was set, enable scissor and set correct values
		if (mClipRect.hasWidth() && mClipRect.hasHeight())
		{
			opengl::enableScissorTest(false);
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
		mRenderableMesh.unbind();
		opengl::enableScissorTest(false);
	}


	MaterialInstance& RenderableMeshComponentInstance::getMaterialInstance()
	{
		return mRenderableMesh.getMaterialInstance();
	}
} 
