#include "renderableclassifycomponent.h"

// External Includes
#include <entity.h>
#include <renderservice.h>
#include <nap/core.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mathutils.h>
#include <nap/datetime.h>

// nap::renderablecopymeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderableClassifyComponent)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableClassifyComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorUniform",		&nap::RenderableClassifyComponent::mColorUniform,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SphereMesh",			&nap::RenderableClassifyComponent::mSphereMesh,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClassifyComponent",	&nap::RenderableClassifyComponent::mClassifyComponent,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PlaneComponent",		&nap::RenderableClassifyComponent::mPlaneComponent,				nap::rtti::EPropertyMetaData::Required)
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
		mColorUniform = extractUniform<UniformVec3>(resource->mColorUniform, mMaterialInstance, errorState);
		if (mColorUniform == nullptr)
			return false;

		// Get handle to matrices, which we set in the draw call
		mProjectionUniform = extractUniform<UniformMat4>("projectionMatrix", mMaterialInstance, errorState);
		if (mProjectionUniform == nullptr)
			return false;
		
		mViewUniform = extractUniform<UniformMat4>("viewMatrix", mMaterialInstance, errorState);
		if (mViewUniform == nullptr)
			return false;
		
		mModelUniform = extractUniform<UniformMat4>("modelMatrix", mMaterialInstance, errorState);
		if (mModelUniform == nullptr)
			return false;

		mBlobCountUniform = extractUniform<UniformInt>("blobCount", mPlaneComponent->getMaterialInstance(), errorState);
		if (mBlobCountUniform == nullptr)
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

		// Add the colors that are randomly picked for every mesh that is drawn
		mColors.emplace_back(RGBColor8(0x5D, 0x5E, 0x73).convert<RGBColorFloat>());
		mColors.emplace_back(RGBColor8(0x8B, 0x8C, 0xA0).convert<RGBColorFloat>());
		mColors.emplace_back(RGBColor8(0xC8, 0x69, 0x69).convert<RGBColorFloat>());
			
		return true;
	}


	void RenderableClassifyComponentInstance::update(double deltaTime)
	{
		// Get global transform
		const glm::mat4x4& model_matrix = mTransform->getGlobalTransform();

		// Get all classified blobs
		std::vector<math::Rect> blobs = mClassifyComponent->getObjects();

		// Get plane material, needs blob information to draw fake shadows
		nap::MaterialInstance& plane_material = mPlaneComponent->getMaterialInstance();

		// First map the blob data from 2D (image) to 3D (scene) 
		// The location is mapped on the x/y plane, size is used to offset on the z-axis
		// This mapped information becomes the new blob target position and size.
		// Limit amount of blobs to 20 (as defined in shader, could be a property)
		int blob_count = math::min<int>(blobs.size(), 20);
		for (int i = 0; i < blob_count; i++)
		{
			// Compute size (radius)
			float size = blobs[i].getHeight() / 2.0f;

			// Compute center of blob in 3D -> object space
			glm::vec3 center
			(
				blobs[i].getMin().x + (blobs[i].getWidth()  / 2.0),
				blobs[i].getMin().y + (blobs[i].getHeight() / 2.0),
				-size
			);

			// Convert location of blob to world space
			glm::mat4 world_loc = glm::translate(model_matrix, center);
			center = math::extractPosition(world_loc);

			// Add new blob if not present
			if (mBlobs.size() <= i)
				mBlobs.emplace_back(Blob(center, size));
			mBlobs[i].set(center, size);
		}

		// Update all 3D blobs so they move and grow towards their previously set target value.
		// Also set blob information in material and store updated (most recent) blob location.
		mLocations.clear();
		for (int i = 0; i < mBlobs.size(); i++)
		{
			// Skip setting location of blob if it has not received a new tracking value.
			if (mBlobs[i].getElapsedTime() > 1.0)
				continue;

			// Update blob and store corrent location and size as a vec4
			glm::vec4 new_location = mBlobs[i].update(deltaTime);

			// Set current blob location in plane material
			std::string center_uniform_name = utility::stringFormat("blobs[%d].mCenter", i);
			assert(plane_material.getMaterial()->findUniform(center_uniform_name) != nullptr);

			UniformVec3& center_uniform = plane_material.getOrCreateUniform<UniformVec3>(center_uniform_name);
			center_uniform.setValue(glm::vec3(new_location));

			// Set current blob size in plane material
			std::string size_uniform_name = utility::stringFormat("blobs[%d].mSize", i);
			assert(plane_material.getMaterial()->findUniform(size_uniform_name) != nullptr);

			UniformFloat& size_uniform = plane_material.getOrCreateUniform<UniformFloat>(size_uniform_name);
			size_uniform.setValue(new_location.w);
			
			// Valid location
			mLocations.emplace_back(new_location);
		}

		// Update number of detected blobs
		mBlobCountUniform->setValue(mLocations.size());
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
		for (auto i = 0; i < mLocations.size(); i++)
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

			// Calculate model matrix, set and push
			glm::mat4 object_loc = glm::translate(glm::mat4(), glm::vec3(mLocations[i]));
			object_loc = glm::scale(object_loc, { mLocations[i].w, mLocations[i].w, mLocations[i].w });
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


	//////////////////////////////////////////////////////////////////////////
	// Blob
	//////////////////////////////////////////////////////////////////////////

	Blob::Blob(const glm::vec3& position, float size)
	{
		// Store position
		mTargetPosition = position;
		mTargetSize = size;

		// Reset smooth operators to construction value
		mPositionOperator.setValue(position);
		mSizeOperator.setValue(size);

		// Reset time
		mTimer.reset();
	}


	void Blob::set(const glm::vec3& position, float size)
	{
		mTargetPosition = position;
		mTargetSize = size;
		mTimer.reset();
	}


	glm::vec4 Blob::update(double deltaTime)
	{
		glm::vec3 new_pos = mPositionOperator.update(mTargetPosition, deltaTime);
		float new_size = mSizeOperator.update(mTargetSize, deltaTime);
		return glm::vec4(new_pos, new_size);
	}


	double Blob::getElapsedTime() const
	{
		return mTimer.getElapsedTime();
	}
}