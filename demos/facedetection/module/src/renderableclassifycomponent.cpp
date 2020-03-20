#include "renderableclassifycomponent.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>

// nap::renderablecopymeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderableClassifyComponent)
	RTTI_PROPERTY("Scale",				&nap::RenderableClassifyComponent::mScale,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableClassifyComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorUniform",		&nap::RenderableClassifyComponent::mColorUniform,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SphereMesh",			&nap::RenderableClassifyComponent::mSphereMesh,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClassifyComponent",	&nap::RenderableClassifyComponent::mClassifyComponent,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::renderablecopymeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableClassifyComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderableClassifyComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	/**
	 * Initializes this component. For this component to work a reference mesh + at least one mesh to copy onto it is needed.
	 * It also makes sure various uniforms (such as color) are present in the material. These uniforms are set when onRender() is called.
	 * But most importantly: it creates a valid RenderableMesh for every mesh to copy and caches it internally.
	 * The renderable mesh represents the coupling between a mesh and material. When valid, the mesh can be rendered with the material.
	 */
	bool RenderableClassifyComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableClassifyComponent* resource = getComponent<RenderableClassifyComponent>();

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

		// Fetch render service
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<RenderService>();

		// Iterate over the meshes to copy
		// Create a valid mesh / material combination based on our referenced material and the meshes to copy
		// If a renderable mesh turns out to be invalid it means that the material / mesh combination isn't valid, ie:
		// There are required vertex attributes in the shader that the mesh doesn't have.
		mSphereMesh = render_service->createRenderableMesh(*resource->mSphereMesh, mMaterialInstance, errorState);
		if (!errorState.check(mSphereMesh.isValid(), "%s, mesh: %s can't be copied", resource->mID.c_str(), resource->mSphereMesh->mID.c_str()))
			return false;

		// Copy over parameters
		mScale	= resource->mScale;

		// Add the colors that are randomly picked for every mesh that is drawn
		mColors.emplace_back(RGBColor8(0x5D, 0x5E, 0x73).convert<RGBColorFloat>());
		mColors.emplace_back(RGBColor8(0x8B, 0x8C, 0xA0).convert<RGBColorFloat>());
		mColors.emplace_back(RGBColor8(0xC8, 0x69, 0x69).convert<RGBColorFloat>());
			
		return true;
	}


	void RenderableClassifyComponentInstance::update(double deltaTime)
	{
		//mTime += (deltaTime * (double)mRotationSpeed);
	}


	nap::MaterialInstance& RenderableClassifyComponentInstance::getMaterial()
	{
		return mMaterialInstance;
	}


	/**
	 * Called by the render service when the app wants to draw this component.
	 * A randomly selected mesh is rendered at the position of every vertex in the reference mesh.
	 * You can change the meshes that are copied and the reference mesh in the JSON file.
	 */
	void RenderableClassifyComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get global transform
		const glm::mat4x4& model_matrix = mTransform->getGlobalTransform();

		// Bind material
		mMaterialInstance.bind();

		// Set non changing uniforms
		mViewUniform->setValue(viewMatrix);
		mProjectionUniform->setValue(projectionMatrix);
		mColorUniform->setValue({ 1.0,0.0,0.0 });

		// Prepare blending
		mMaterialInstance.pushBlendMode();

		// Fix seed for subsequent random calls
		math::setRandomSeed(mSeed);

		// Get randomization scale for various effects
		int max_rand_color = static_cast<int>(mColors.size()) - 1;

		// Push all existing uniforms to GPU
		mMaterialInstance.pushUniforms();

		// Fetch the uniform declarations of the uniform values we want to update in the copy loop
		// This allows us to only push a specific uniform instead of all uniforms.
		// Every uniform maps to a declaration, where there can be multiple values associated with a single declaration.
		// You can look at the uniform declaration as the actual slot on a shader that accepts a value of a specific type.
		const auto& color_binding = mMaterialInstance.getUniformBinding(mColorUniform->mName);
		const auto& objec_binding = mMaterialInstance.getUniformBinding(mModelUniform->mName);

		// Iterate over every point, fetch random mesh, construct custom object matrix, set uniforms and render.
		std::vector<math::Rect> blobs = mClassifyComponent->getObjects();
		mLocations.clear();
		mSizes.clear();
		for (auto i = 0; i < blobs.size(); i++)
		{			
			// Pick random color for mesh and push to GPU
			glm::vec3 color = mColors[math::random<int>(0, max_rand_color)].toVec3();
			mColorUniform->setValue(color);
			mColorUniform->push(*color_binding.mDeclaration);

			// Get the mesh to stamp onto this point and bind
			RenderableMesh& render_mesh = mSphereMesh;
			render_mesh.bind();

			// Get data of that mesh
			MeshInstance& mesh_instance = render_mesh.getMesh().getMeshInstance();

			// GPU mesh representation of mesh to copy
			opengl::GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();

			// Get size and center from rect
			glm::vec3 center
			(
				blobs[i].getMin().x + (blobs[i].getWidth() / 2.0),
				blobs[i].getMin().y + (blobs[i].getHeight() / 2.0),
				0
			);
			float size = blobs[i].getHeight() / 2.0f;
			mSizes.emplace_back(size);

			// Calculate model matrix and store
			glm::mat4x4 object_loc = glm::translate(model_matrix, center + glm::vec3(0.0f, 0.0f, -size));
			object_loc = glm::scale(object_loc, { size, size, size });
			mLocations.emplace_back(object_loc);

			// Add scale, set as value and push
			mModelUniform->setValue(object_loc);
			mModelUniform->push(*objec_binding.mDeclaration);

			// Iterate over all the shapes and render
			for (int shape_idx = 0; shape_idx < mesh_instance.getNumShapes(); ++shape_idx)
			{
				// Get the shape we want to draw and the index buffer associated with that shape
				MeshShape& shape = mesh_instance.getShape(shape_idx);
				const opengl::IndexBuffer& index_buffer = gpu_mesh.getIndexBuffer(shape_idx);

				GLenum draw_mode = getGLMode(shape.getDrawMode());
				GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());

				// Bind and draw all the currently attached vbo's based on the shape's indices.
				index_buffer.bind();
				glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
				index_buffer.unbind();
			}
			// Unbind this instance
			render_mesh.unbind();
		}

		// Unbind material
		mMaterialInstance.unbind();
	}

}