#include "renderablecopymeshcomponent.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <renderservice.h>
#include <nap/core.h>
#include <materialutils.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>

// nap::renderablecopymeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderableCopyMeshComponent)
	RTTI_PROPERTY("Orient",				&nap::RenderableCopyMeshComponent::mOrient,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale",				&nap::RenderableCopyMeshComponent::mScale,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RandomScale",		&nap::RenderableCopyMeshComponent::mRandomScale,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableCopyMeshComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorUniform",		&nap::RenderableCopyMeshComponent::mColorUniform,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TargetMesh",			&nap::RenderableCopyMeshComponent::mTargetMesh,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CopyMeshes",			&nap::RenderableCopyMeshComponent::mCopyMeshes,					nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::renderablecopymeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableCopyMeshComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderableCopyMeshComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	bool RenderableCopyMeshComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableCopyMeshComponent* resource = getComponent<RenderableCopyMeshComponent>();

		// Fetch transform, used to offset the copied meshes
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if(!errorState.check(mTransform != nullptr, 
			"%s: unable to find transform component", resource->mID.c_str()))
			return false;

		// Initialize our material instance based on values in the resource
		if (!mMaterialInstance.init(resource->mMaterialInstanceResource, errorState))
			return false;

		// Get handle to color uniform, which we set in the draw call
		mColorUniform = extractUniform<UniformVec3>(resource->mColorUniform, errorState);
		if (mColorUniform == nullptr)
			return false;

		// Get handle to matrices, which we set in the draw call
		mProjectionUniform = extractUniform<UniformMat4>("projectionMatrix", errorState);
		if (mProjectionUniform == nullptr)
			return false;
		mViewUniform = extractUniform<UniformMat4>("viewMatrix", errorState);
		if (mViewUniform == nullptr)
			return false;
		mModelUniform = extractUniform<UniformMat4>("modelMatrix", errorState);
		if (mModelUniform == nullptr)
			return false;

		// Ensure there's at least 1 mesh to copy
		if (!errorState.check(!(resource->mCopyMeshes.empty()), 
			"no meshes found to copy: %s", resource->mID.c_str()))
			return false;

		// Fetch render service
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<RenderService>();

		// Iterate over the meshes to copy
		// Create a valid mesh / material combination based on our referenced material and the meshes to copy
		// If a renderable mesh turns out to be invalid it means that the material / mesh combination isn't valid, ie:
		// There are required vertex attributes in the shader that the mesh doesn't have.
		for(auto& mesh : resource->mCopyMeshes)
		{
			RenderableMesh render_mesh = render_service->createRenderableMesh(*mesh, mMaterialInstance, errorState);
			if (!errorState.check(render_mesh.isValid(), "%s, mesh: %s can't be copied", resource->mID.c_str(), mesh->mID.c_str()))
				return false;

			// Store renderable mesh
			mCopyMeshes.emplace_back(render_mesh);
		}

		// Store handle to target mesh
		mTargetMesh = resource->mTargetMesh.get();

		// Ensure the vertices are valid
		mTargetVertices = mTargetMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		if (!errorState.check(mTargetVertices != nullptr, "%s: unable to find target vertex position attribute", resource->mID.c_str()))
			return false;

		// Ensure the normals are valid
		mTargetNormals = mTargetMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		if (!errorState.check(mTargetNormals != nullptr, "%s: unable to find target normal attribute", resource->mID.c_str()))
			return false;

		// Copy orientation and scale
		mOrient = resource->mOrient;
		mScale	= resource->mScale;
		mRandomScale = resource->mRandomScale;

		return true;
	}


	void RenderableCopyMeshComponentInstance::update(double deltaTime)
	{

	}


	nap::MaterialInstance& RenderableCopyMeshComponentInstance::getMaterial()
	{
		return mMaterialInstance;
	}


	void RenderableCopyMeshComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get global transform
		const glm::mat4x4& model_matrix = mTransform->getGlobalTransform();

		// Get material to bind
		Material* comp_mat = mMaterialInstance.getMaterial();
		comp_mat->bind();

		// Set non changing uniforms
		mViewUniform->setValue(viewMatrix);
		mProjectionUniform->setValue(projectionMatrix);
		mColorUniform->setValue({ 1.0,0.0,0.0 });

		// Prepare blending
		utility::setBlendMode(mMaterialInstance);

		// Get points to copy onto
		std::vector<glm::vec3>& pos_data = mTargetVertices->getData();
		std::vector<glm::vec3>& nor_data = mTargetNormals->getData();

		// Fix seed for subsequent random calls
		math::setRandomSeed(mSeed);

		// Clamp random scale value
		float rand_scale = math::clamp<float>(mRandomScale, 0.0f, 1.0f);

		// Iterate over every point, construct custom object matrix
		// And render
		for (auto i = 0; i < pos_data.size(); i++)
		{
			// Pick number
			int mesh_idx = math::random<int>(0, mCopyMeshes.size() - 1);
			glm::vec3 color = math::random<glm::vec3>({ 0.0,0.0,0.0 }, { 1.0,1.0,1.0 });
			mColorUniform->setValue(color);

			// Get the mesh to stamp onto this point and bind
			RenderableMesh& render_mesh = mCopyMeshes[mesh_idx];
			render_mesh.bind();

			// Get data of that mesh
			MeshInstance& mesh_instance = render_mesh.getMesh().getMeshInstance();

			// GPU mesh representation of mesh to copy
			opengl::GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();

			// Calculate model matrix
			glm::mat4x4 object_loc = glm::translate(model_matrix, pos_data[i]);

			// Orient using normal
			if (mOrient)
			{
				glm::vec3 nor_normal = glm::normalize(nor_data[i]);
				glm::vec3 rot_normal = glm::cross({ 0,1,0 }, glm::normalize(nor_normal));
				float rot_value = glm::acos(glm::dot({ 0,1,0 }, nor_normal));
				object_loc = glm::rotate(object_loc, rot_value, rot_normal);
			}

			// Add scale and set as object matrix
			float fscale = math::random<float>(1.0f - rand_scale, 1.0f) * mScale;
			mModelUniform->setValue(glm::scale(object_loc, { fscale, fscale, fscale }));

			// Push uniforms to gpu
			utility::pushUniforms(mMaterialInstance);

			// Iterate over all the shapes and render
			for (int shape_idx = 0; shape_idx < mesh_instance.getNumShapes(); ++shape_idx)
			{
				MeshShape& shape = mesh_instance.getShape(shape_idx);
				const opengl::IndexBuffer& index_buffer = gpu_mesh.getIndexBuffer(shape_idx);

				GLenum draw_mode = getGLMode(shape.getDrawMode());
				GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());

				index_buffer.bind();
				glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
				index_buffer.unbind();
			}
			
			render_mesh.unbind();
		}

		// Unbind material
		comp_mat->unbind();
	}

}