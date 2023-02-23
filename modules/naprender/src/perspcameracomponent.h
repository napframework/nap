/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "cameracomponent.h"

namespace nap
{
	// Forward Declares
	class PerspCameraComponentInstance;
	class TransformComponentInstance;
	class TransformComponent;

	/**
	 * Perspective camera properties.
	 */
	struct NAPAPI PerspCameraProperties
	{
		float mFieldOfView			= 50.0f;				///< Property: "FieldOfView" perspective camera field of view
		float mNearClippingPlane	= 1.0f;					///< Property: "NearClippingPlane" camera near clipping plane
		float mFarClippingPlane		= 1000.0f;				///< Property: "FarClippingPlane" camera far clipping plane
		glm::ivec2 mGridDimensions	= glm::ivec2(1, 1);		///< Property: "GridDimensions" of 'split projection' grid. Default is single dimension, meaning a single screen, which is a regular symmetric perspective projection
		glm::ivec2 mGridLocation	= glm::ivec2(0, 0);		///< Property: "GridLocation" the 2 dimensional index in the split projection dimensions
	};
	

	/**
	 * Perspective camera component resource. Holds static data as read from file.
	 */
	class NAPAPI PerspCameraComponent : public CameraComponent
	{
		RTTI_ENABLE(CameraComponent)
		DECLARE_COMPONENT(PerspCameraComponent, PerspCameraComponentInstance)
	public:
		PerspCameraProperties mProperties;					///< Property: 'Properties' the perspective camera settings
	};


	/**
	 * Implementation of the perspective camera. 
	 * The view matrix is calculated using the transform attached to the entity. 
	 */
	class NAPAPI PerspCameraComponentInstance : public CameraComponentInstance
	{
		RTTI_ENABLE(CameraComponentInstance)
	public:
		// Default constructor
		PerspCameraComponentInstance(EntityInstance& entity, Component& resource);

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
		 * @return The populated view matrix.
		 */
		virtual const glm::mat4 getViewMatrix() const override;

		/**
		 * Use this function to split the projection into a regular grid. This can be used to render to multiple screens
		 * with a single camera. Use setGridLocation to set the horizontal and vertical index into this grid.
		 * @param numRows the number of cells in the horizontal
		 * @param numColumns the number of cells in the vertical direction.
		 */
		void setGridDimensions(int numRows, int numColumns);

		/**
		* Sets the horizontal and vertical index into the projection grid as set by setGridDimensions.
		*/
		void setGridLocation(int row, int column);

		/**
		 * Sets the fov parameter
		 * @param fov the new field of view
		 */
		void setFieldOfView(float fov);

		/**
		 * @return camera field of view
		 */
		float getFieldOfView() const;

		/**
		 * @return camera near clipping plane
		 */
		float getNearClippingPlane() const;

		/**
		 * @return camera far clipping plane
		 */
		float getFarClippingPlane() const;

		/**
		 * Returns the matrix that is used to transform a 3d scene in to a 2d projection by the renderer.
		 * Vulkan uses a coordinate system where (-1, -1) is in the top left quadrant, instead of the bottom left quadrant.
		 * Use this matrix, instead of the one returned by getProjectionMatrix(), when an ortographic projection matrix is required as shader input.
		 * For all regular (CPU) related orthographic calculations, use getProjectionMatrix().
		 * @return the projection matrix used by the renderer
		 */
		virtual const glm::mat4& getRenderProjectionMatrix() const override;

		/**
		 * @return the perspective camera properties
		 */
		PerspCameraProperties getProperties() const;

		/**
		 * Sets the camera instance properties
		 */
		void setProperties(const PerspCameraProperties& props);

	private:

		/**
		 * Recomputes the projection matrix when requested the next time
		 */
		void setDirty() { mDirty = true; }

		/**
		 * Updates all projection matrices when dirty
		 */
		void updateProjectionMatrices() const;

	protected:
		mutable glm::mat4x4				mProjectionMatrix;						// The composed projection matrix
		mutable glm::mat4x4				mRenderProjectionMatrix;				// The composed projection matrix used by the renderer
		mutable bool					mDirty = true;							// If the projection matrix needs to be recalculated
		PerspCameraProperties			mProperties;							// These properties are copied from the resource to the instance. When these are changed, only the instance is affected
		TransformComponentInstance*		mTransformComponent;					// Cached transform component

		bool							mPerpendicularRenderProjection = true;	// Whether the render projection matrix should be projected onto a perpendicular surface
	};
}
