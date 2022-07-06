/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "cameracomponent.h"
#include "lookingglassdevice.h"

namespace nap
{
	// Forward Declares
	class QuiltCameraComponentInstance;
	class TransformComponentInstance;

	/**
	 * Quilt camera properties.
	 */
	struct NAPAPI QuiltCameraProperties
	{
		float mNearClippingPlane = 1.0f;					///< Property: 'NearClippingPlane' camera near clipping plane
		float mFarClippingPlane = 1000.0f;					///< Property: 'FarClippingPlane' camera far clipping plane
		float mCameraSize = 1.0f;							///< Property: 'CameraSize' the vertical radius of the focal plane
	};


	/**
	 * Quilt camera component resource. Holds static data as read from file.
	 *
	 * The quilt camera is a perspective camera that uses an off-axis projection over a horizontal offset within a 
	 * specified view cone. The offset of the camera can be modified with QuiltCameraComponentInstance::setView().
	 * The size of the view cone is determined by the Looking Glass device calibration.
	 *
	 * When used in combination with a nap::QuiltRenderTarget, a quilt texture can be rendered. A quilt is a large
	 * texture that includes a grid of subtextures, each presenting a view into the scene from a slightly different
	 * perspective. Quilts are in turn fed to nap::LightFieldShader to render a light field that can be displayed
	 * on the Looking Glass.
	 * 
	 * See: https://docs.lookingglassfactory.com/keyconcepts/camera
	 */
	class NAPAPI QuiltCameraComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(QuiltCameraComponent, QuiltCameraComponentInstance)

		/**
		 * The quilt camera needs on a transform to calculate it's view matrix
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

	public:
		ResourcePtr<LookingGlassDevice> mDevice;			///< Property: 'LookingGlassDevice' the looking glass device
		QuiltCameraProperties mProperties;					///< Property: 'Properties' the quilt camera settings
	};


	/**
	 * Implementation of the quilt camera. 
	 * The view matrix is calculated using the transform attached to the entity. 
	 */
	class NAPAPI QuiltCameraComponentInstance : public CameraComponentInstance
	{
		RTTI_ENABLE(CameraComponentInstance)
	public:
		// Default constructor
		QuiltCameraComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 * @param errorState contains the error if the camera can't be initialized properly.
		 * @return if the camera initialized properly.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Extracts the size in pixels of the render target. 
		 * The dimensions are used to calculate the correct projection matrix.
		 * @param size The size of the render target in pixel coordinates.
		 */
		virtual void setRenderTargetSize(const glm::ivec2& size) override;

		/**
		 * Returns the matrix that is used to transform a 3d scene into a 2d projection.
		 * Use this matrix for all regular CPU side projection calculations.
		 * Use the matrix returned by getRenderProjectionMatrix() as shader input only.
		 * @return camera projection matrix
		 */
		virtual const glm::mat4& getProjectionMatrix() const override;

		/**
		 * Returns a generated view matrix based on the current view index and count.
		 * @return The populated view matrix.
		 */
		virtual const glm::mat4 getViewMatrix() const override;

		/**
		 * @return current field of view in radians
		 */
		float getFieldOfView() const;

		/**
		 * @return the distance between the camera and the focal plane
		 */
		float getDistanceToFocalPlane() const;

		/**
		 * @return the optimal focal point of the camera in world space.
		 */
		glm::vec3 getFocalPoint() const;

		/**
		 * @return the offset from the focal point to the centered camera position.
		 */
		glm::vec3 getFocalOffset() const;

		/**
		 * Sets the index of the current view. The viewpoint is calculated as a fraction over the
		 * horiozontal offset (a line parallel to the focal plane) of the viewcone from left to right.
		 *
		 * @param view the view index, where the first view is zero and the last is viewCount-1
		 * @param viewCount the number of views
		 */
		void setView(int view, int viewCount);

		/**
		 * Resets the current view and viewcount so that it is aligned with the center of the focal plane.
		 * Essentially makes this camera component behave as 'nap::PerspectiveCamera'.
		 */
		void resetView();

		/**
		 * Sets the size of the camera.
		 * See: https://docs.lookingglassfactory.com/keyconcepts/camera#offset
		 */
		void setCameraSize(float size);


		/**
		 * Returns the size of the camera.
		 * See: https://docs.lookingglassfactory.com/keyconcepts/camera#offset
		 */
		float getCameraSize() const;

		/**
		 * Returns the matrix that is used to transform a 3d scene in to a 2d projection by the renderer.
		 * Vulkan uses a coordinate system where (-1, -1) is in the top left quadrant, instead of the bottom left quadrant.
		 * Use this matrix, instead of the one returned by getProjectionMatrix(), when an ortographic projection matrix is required as shader input.
		*  For all regular (CPU) related orthographic calculations, use getProjectionMatrix().
		 * @return the projection matrix used by the renderer
		 */
		virtual const glm::mat4& getRenderProjectionMatrix() const override;

	private:
		/**
		 * Set the view cone in degrees.
		 *
		 * The Looking Glass has a valid viewing angle of about 40 - 50° total, or 20 - 25° in each direction from center.
		 * Due to refractive properties of the glass itself, the digital view angle that we find that renders best is 35°.
		 */
		void setViewCone(float degrees);

		/**
		 * Recomputes the projection matrix when requested the next time
		 */
		void setDirty() { mDirty = true; }

		/**
		 * Updates all view and projection matrices when dirty
		 */
		void updateMatrices() const;

	protected:
		mutable glm::mat4x4				mProjectionMatrix;						// The composed projection matrix
		mutable glm::mat4x4				mRenderProjectionMatrix;				// The composed projection matrix used by the renderer
		mutable bool					mDirty = true;							// If the projection matrix needs to be recalculated

		QuiltCameraProperties			mProperties;							// These properties are copied from the resource to the instance. When these are changed, only the instance is affected
		TransformComponentInstance*		mTransformComponent;					// Cached transform component

		mutable float					mDistanceToFocal = 0.0f;				// Distance from camera to focal plane or "zero-parallax plane"
		mutable float					mHorizontalOffset = 0.0f;

		float							mViewCone = glm::radians(40.0f);		// View cone in radians
		int								mCurrentViewIndex = 0;
		int								mViewCount = 1;

		bool							mPerpendicularRenderProjection = true;	// Whether the render projection matrix should be projected onto a perpendicular surface
	};
}
