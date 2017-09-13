#pragma once

#include "cameracomponent.h"

namespace nap
{
	class PerspCameraComponentInstance;
	class TransformComponentInstance;
	class TransformComponent;

	/**
	 * Properties of the perspective camera, used in both the camera resource and instance.
	 */
	struct NAPAPI PerpCameraProperties
	{
		float mFieldOfView			= 50.0f;				// Field of View
		float mNearClippingPlane	= 1.0f;					// Near clipping plane
		float mFarClippingPlane		= 1000.0f;				// Far clipping plane

		glm::ivec2 mGridDimensions	= glm::ivec2(1, 1);		// Dimensions of 'split projection' grid. Default is single dimension, meaning a single screen, which is a regular symmetric perspective projection
		glm::ivec2 mGridLocation	= glm::ivec2(0, 0);		// Location means the 2 dimensional index in the split projection dimensions
	};
	
	/**
	 * Resource class for the perspective camera. Holds static data as read from file.
	 */
	class NAPAPI PerspCameraComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PerspCameraComponent, PerspCameraComponentInstance)

		/**
		 * Camera is dependent on the transform component for calculating the view matrix.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override { components.push_back(RTTI_OF(TransformComponent)); }

	public:
		PerpCameraProperties mProperties;	// Properties of the camera
	};

	/**
	 * Implementation of the perspective camera. View matrix is calculated from the transform component
	 * that is attached to the entity. Be sure to call setRenderTargetSize to use the correct aspect ratio.
	 */
	class NAPAPI PerspCameraComponentInstance : public CameraComponentInstance
	{
		RTTI_ENABLE(CameraComponentInstance)
	public:
		// Default constructor
		PerspCameraComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * This implementation extracts the size in pixels of the render target to make sure that the orthographic
		 * camera acts in pixel coordinates.
		 * @param size The size of the render target in pixel coordinates.
		 */
		virtual void setRenderTargetSize(glm::ivec2 size) override;

		/**
	 	 * @return camera projection matrix
		 * Use this matrix to transform a 3d scene in to a 2d projection
		 */
		virtual const glm::mat4& getProjectionMatrix() const override;

		/**
		 * @return The populated view matrix.
		 */
		virtual const glm::mat4 getViewMatrix() const override;

		/**
		* Use this function to split the projection into a grid of squares. This can be used to render to multiple screens 
		* with a single camera. Use setGridLocation to set the horizontal and vertical index into this grid.
		* @param splitDimensions: the number of cells in the horizontal and vertical direction.
		*/
		void setGridDimensions(int numRows, int numColumns);

		/**
		* Sets the horizontal and vertical index into the projection grid as set by setSplitDimensions.
		*/
		void setGridLocation(int row, int column);

	private:
		/**
		* Sets this camera to be dirty, ie:
		* next time the matrix is queried it is recomputed
		*/
		void setDirty() { mDirty = true; }

	private:
		mutable glm::mat4x4		mProjectionMatrix;		// The composed projection matrix

		mutable bool			mDirty = true;			// If the projection matrix needs to be recalculated
		glm::ivec2				mRenderTargetSize;		// The size of the render target we're rendering to
		
		PerpCameraProperties	mProperties;			// These properties are copied from the resource to the instance. When these are changed, only the instance is affected
		TransformComponentInstance*		mTransformComponent;	// Cached transform component
	};
}
