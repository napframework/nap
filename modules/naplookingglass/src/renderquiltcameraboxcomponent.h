/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "quiltcameracomponent.h"
#include "quiltcameraboxmesh.h"

// External Includes
#include <rendercomponent.h>
#include <renderablemesh.h>
#include <materialinstance.h>
#include <nap/resourceptr.h>
#include <transformcomponent.h>
#include <rect.h>
#include <componentptr.h>

namespace nap
{
	class RenderQuiltCameraBoxComponentInstance;
	
	/**
	 * Resource part of the RenderQuiltCameraBoxComponent.
	 *
	 * Renders a box that is placed and transformed in front of the specified 'QuiltCameraComponent' such that
	 * its faces extend the edges of the camera focal plane into the forward direction of the camera, and therefore,
	 * the looking glass. Great for demonstrating the parallax effect and visualizing the far plane. Note that increasing
	 * the box depth too much will bring the box rear out of focus. Front-culling is enabled on the box mesh so that only
	 * the inside is visible. 
	 *
	 * The transform instance of this object is controlled programmatically based on the reference camera.Therefore, it
	 * does not truly exist in the scene to render like other objects. Using parent transforms that perform some
	 * modification on the location or orientation of this object can cause undesired behavior. 
	 */
	class NAPAPI RenderQuiltCameraBoxComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderQuiltCameraBoxComponent, RenderQuiltCameraBoxComponentInstance)
	public:
		/**
		 * RenderableMesh requires a transform to position itself in the world.
		 * @param components the components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

	public:
		ComponentPtr<QuiltCameraComponent>	mQuiltCameraComponent;				///< Property: 'QuiltCameraComponent' Reference quilt camera.
		MaterialInstanceResource			mMaterialInstanceResource;			///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance.
		EDrawMode						mPolygonMode = EDrawMode::Lines;	///< Property: 'PolygonMode' The polygon mode to use.
		float								mLineWidth = 1.0f;					///< Property: 'LineWidth' Width of the line when rendered, values higher than 1.0 only work when the GPU supports it.
		float								mBoxDepth = 1.0f;					///< Property: 'BoxDepth' The z-extent of the box in camera space.
	};


	/**
	 * Instance part of the RenderQuiltCameraBoxComponent.
	 *
	 * Renders a box that is placed and transformed in front of the specified 'QuiltCameraComponent' such that
	 * its faces extend the edges of the camera focal plane into the forward direction of the camera, and therefore,
	 * the looking glass. Great for demonstrating the parallax effect and visualizing the far plane. Note that increasing
	 * the box depth too much will bring the box rear out of focus. Front-culling is enabled on the box mesh so that only
	 * the inside is visible.
	 *
	 * The transform instance of this object is controlled programmatically based on the reference camera.Therefore, it
	 * does not truly exist in the scene to render like other objects. Using parent transforms that perform some
	 * modification on the location or orientation of this object can cause undesired behavior.
	 */
	class NAPAPI RenderQuiltCameraBoxComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)

	public:
		RenderQuiltCameraBoxComponentInstance(EntityInstance& entity, Component& component);

		/**
		 * Initializes this component.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Updates the local transform of this component such that it represents the location of the back plane. 
		 * This ensures the depth sorter considers this object to be behind other components inside the box.
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return current material used when drawing the mesh.
		 */
		MaterialInstance& getMaterialInstance();

		/**
		 * @return Currently active mesh that is drawn.
		 */
		IMesh& getMesh()										{ return mRenderableMesh.getMesh(); }

		/**
		 * Returns the runtime version of the mesh that is drawn, part of the original mesh.
		 * @return the mesh instance that is drawn
		 */
		MeshInstance& getMeshInstance()							{ return getMesh().getMeshInstance(); }

		/**
		* Sets line width
		* @param lineWidth New line width
		*/
		void setLineWidth(float lineWidth)						{ mLineWidth = lineWidth; }

		/**
		 * @return the transform component instance, used to compose the model matrix
		 */
		const TransformComponentInstance& getTransform()		{ return *mTransformComponent; }

		/**
		 * Called by the Render Service. This component only supports `nap::QuiltCameraComponentInstance`.
		 * @return if the object can be rendered with the given camera
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override;

		/**
		 * Sets the box depth.
		 * @param depth the new depth of the box in camera space.
		 */
		void setBoxDepth(float depth)							{ mBoxDepth = depth; }

		/**
		* @return the depth of the box in camera space.
		*/
		float getBoxDepth() const								{ return mBoxDepth; }

		ComponentInstancePtr<QuiltCameraComponent> mQuiltCameraComponent = { this, &RenderQuiltCameraBoxComponent::mQuiltCameraComponent };

	protected:
		/**
		 * Creates a renderable mesh that can be used to switch to another mesh and/or material at runtime. This function should be called from
		 * init() functions on other components, and the result should be validated.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param materialInstance The material instance that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState);

		/**
		 * Creates a renderable mesh that can be used to switch to another mesh at runtime. The material remains the same material as the one that
		 * was already set on the RenderQuiltCameraBoxComponent.. This function should be called from init() functions on other components, and the result
		 * should be validated.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, utility::ErrorState& errorState);

		/**
		 * Switches the mesh and/or the material that is rendered. The renderable mesh should be created through createRenderableMesh, and must
		 * be created from an init() function.
		 * @param mesh The mesh that was retrieved through createRenderableMesh.
		 */
		void setMesh(const RenderableMesh& mesh);

		/**
		 * Renders the model from the ModelResource, using the material on the ModelResource.
	 	 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		TransformComponentInstance*				mTransformComponent;			///< Cached pointer to transform
		MaterialInstance						mMaterialInstance;				///< The MaterialInstance as created from the resource. 
		RenderableMesh							mRenderableMesh;				///< The currently active renderable mesh, either set during init() or set by setMesh.
		RenderService*							mRenderService = nullptr;		///< Pointer to the renderer
		UniformMat4Instance*					mModelMatUniform = nullptr;		///< Pointer to the model matrix uniform
		UniformMat4Instance*					mViewMatUniform = nullptr;		///< Pointer to the view matrix uniform
		UniformMat4Instance*					mProjectMatUniform = nullptr;	///< Pointer to the projection matrix uniform
		float									mLineWidth = 1.0f;				///< Line width, clamped to 1.0 if not supported by GPU
		float									mBoxDepth = 1.0f;				///< The z-extent of the box in camera space

		std::unique_ptr<QuiltCameraBoxMesh>		mBoxMesh;
	};
}
