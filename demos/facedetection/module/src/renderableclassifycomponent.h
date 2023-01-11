/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <rendercomponent.h>
#include <renderablemesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <color.h>
#include <spheremesh.h>
#include <cvclassifycomponent.h>
#include <renderablemeshcomponent.h>
#include <smoothdamp.h>
#include <nap/timer.h>
#include <uniforminstance.h>
#include <imguiservice.h>

namespace nap
{
	// Forward Declares
	class RenderService;
	class Core;

	//////////////////////////////////////////////////////////////////////////
	// Blob
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple structure to smoothly interpolate a single blob position and size over time.
	 */
	class Blob final
	{
	public:
		// Constructor
		Blob(const glm::vec3& position, float size);

		// Destructor
		~Blob() = default;

		/**
		 * Sets the values to update the smoothers with on update.
		 * @param position new blob position
		 * @param size new blob size
		 */
		void set(const glm::vec3& position, float size);

		/**
		 * Updates the smoothers without giving a new target and size.
		 * @return vec4 where the first 3 values are position, the 4th size.
		 */
		glm::vec4 update(double deltaTime);

		/**
		 * @return time elapsed tince the blob received new data
		 */
		double getElapsedTime() const;

		glm::vec3 mTargetPosition;															///< Target position
		float mTargetSize = 0.0f;															///< Size

	private:
		math::SmoothOperator<glm::vec3> mPositionOperator = { {0.0f,0.0f,0.0f}, 0.25f };	///< Current blob position
		math::SmoothOperator<float> mSizeOperator = { 0.0f, 0.25f };						///< Current blob size
		nap::SystemTimer mTimer;															///< When the blob received it's last position / size update.
	};


	//////////////////////////////////////////////////////////////////////////
	// Renderable Classify Component
	//////////////////////////////////////////////////////////////////////////

	// Forward declare instance part
	class RenderableClassifyComponentInstance;

	/**
	 * Resource part of the RenderableClassifyComponent.
	 * Places and renders the classified (detected) 2D objects as 3D spheres.
	 */
	class NAPAPI RenderableClassifyComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableClassifyComponent, RenderableClassifyComponentInstance)
	public:

		/**
		 * This component depends on a transform component
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Component Properties
		MaterialInstanceResource mMaterialInstanceResource;			///< Property: 'MaterialInstance' the material used to render the blobs
		std::string mColorUniform = "color";						///< Property: 'ColorUniform' name of the color uniform binding (vec3) in the shader
		ResourcePtr<SphereMesh> mSphereMesh;						///< Property: 'Sphere' sphere mesh to render as blob
		ComponentPtr<CVClassifyComponent> mClassifyComponent;		///< Property: 'ClassifyComponent' components that contains detected objects
		ComponentPtr<RenderableMeshComponent> mPlaneComponent;		///< Property: 'PlaneComponent'component that renders the plane
	};


	/**
	 * Instance part of the RenderableClassifyComponent.
	 * Places and renders the classified (detected) 2D objects as 3D spheres.
	 * 
	 * It also pushes the position (and size) of the detected blobs to the material of the ground plane.
	 * The material of the ground plane uses that information to generate fake shadows.
	 */
	class NAPAPI RenderableClassifyComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderableClassifyComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderableCopyMeshComponentInstance based on the RenderableCopyMeshComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the renderablecopymeshcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @param deltaTime time in between calls in seconds	
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return material used for rendering the copied meshes	
		 */
		MaterialInstance& getMaterial();

		/**
		 * Returns the most recent blob placement and size information, packaged as: position: xyz, size: z
		 * @return list of recent blob placements and size in world space
		 */
		const std::vector<glm::vec4>& getLocations() const				{ return mLocations; }

		/**
		 * Link to the classification component, contains list of detected blobs
		 */
		ComponentInstancePtr<CVClassifyComponent> mClassifyComponent = { this, &RenderableClassifyComponent::mClassifyComponent };

		/**
		 * Link to the component that renders the plane
		 */
		ComponentInstancePtr<RenderableMeshComponent> mPlaneComponent = { this, &RenderableClassifyComponent::mPlaneComponent };

		int		mSeed = 0;											///< Random seed

	protected:
		/**
		* Draws a randomly selected mesh at the position of every vertex in the target mesh.
		* @param viewMatrix the camera world space location
		* @param projectionMatrix the camera projection matrix
		*/
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		/**
		 * Helper function that is used to extract a specific type of uniform out of a material.
		 * First it checks if the material actually has (supports) a uniform with the given name.
		 * If that is the case a new uniform with that name is retrieved.
		 * @param name uniform name
		 * @param ubo the uniform buffer object to extract uniform from.
		 * @param error contains the error if extraction fails
		 * @return the extracted uniform
		 */
		template<typename T>
		T* extractUniform(const std::string& name, nap::UniformStructInstance& ubo, utility::ErrorState& error);

		TransformComponentInstance* mTransform = nullptr;				///< Transform used to position instanced meshes
		RenderableMesh mSphereMesh;										///< Valid mesh / material combinations
		MaterialInstance mMaterialInstance;								///< The MaterialInstance as created from the resource. 
		nap::UniformVec3Instance* mColorUniform = nullptr;				///< Color uniform slot
		nap::UniformMat4Instance* mProjectionUniform = nullptr;			///< Projection matrix uniform slot
		nap::UniformMat4Instance* mViewUniform = nullptr;				///< View matrix uniform slot
		nap::UniformMat4Instance* mModelUniform = nullptr;				///< Model matrix uniform
		nap::UniformMat4Instance* mNormalUniform = nullptr;				///< Normal matrix rotation uniform
		nap::UniformVec3Instance* mCameraPosUniform = nullptr;			///< Camera position uniform
		nap::UniformIntInstance* mBlobCountUniform = nullptr;			///< Number of blobs uniform
		nap::UniformStructArrayInstance* mBlobsUniform = nullptr;		///< Handle to the array of blobs in the plane material
		std::vector<RGBColorFloat> mColors;								///< All selectable colors
		std::vector<Blob> mBlobs;										///< All the blobs in world space
		std::vector<glm::vec4> mLocations;								///< Recent blob location and size data, xyz = position, z = size
		nap::RenderService* mRenderService = nullptr;					///< Handle to the renderer
		nap::IMGuiService* mGUIService = nullptr;						///< Handle to the gui
	};


	template<typename T>
	T* nap::RenderableClassifyComponentInstance::extractUniform(const std::string& name, nap::UniformStructInstance& ubo, utility::ErrorState& error)
	{
		// Get / create uniform if exists
		T* found_uni = ubo.getOrCreateUniform<T>(name);
		if (found_uni == nullptr)
			error.fail("Unable to find uniform: %s", name.c_str());
		return found_uni;
	}

}
