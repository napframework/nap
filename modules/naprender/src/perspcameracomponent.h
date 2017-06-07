#pragma once

#include "cameracomponent.h"

namespace nap
{
	class PerspCameraComponent;
	class TransformComponent;

	struct PerpCameraProperties
	{
		float mFieldOfView = 50.0f;
		float mNearClippingPlane = 1.0f;
		float mFarClippingPlane = 1000.0f;

		glm::ivec2 mGridDimensions = glm::ivec2(1, 1);					// Dimensions of 'split projection' grid. Default is single dimension, meaning a single screen, which is a regular symmetric perspective projection
		glm::ivec2 mGridLocation = glm::ivec2(0, 0);					// Location means the 2 dimensional index in the split projection dimensions
	};
	
	class PerspCameraComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) { components.push_back(RTTI_OF(TransformComponent)); }
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(PerspCameraComponent); }

	public:
		PerpCameraProperties mProperties;
	};

	/**
	 * Acts as a camera in the render system
	 * The camera does not carry a transform and therefore
	 * only defines a projection matrix.
	 */
	class PerspCameraComponent : public CameraComponent
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		PerspCameraComponent(EntityInstance& entity);

		/**
		 * 
		 */
		virtual bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState);

		/**
		* Convenience method to specify lens aspect ratio, defined as width / height
		* @param width, arbitrary width, most often the resolution of the canvas
		* @param height, arbitrary height, most often the resolution of the canvas
		*/
		void	setAspectRatio(float width, float height);

		/**
		* @return camera projection matrix
		* Use this matrix to transform a 3d scene in to a 2d projection
		*/
		virtual const glm::mat4& getProjectionMatrix() const override;

		/**
		 * Returns the view matrix of the camera
		 * The view is determined by a number of factors including the camera's position
		 * and possible look at objects
		 * @return The populated view matrix
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

		/**
		 * Sets this camera to be dirty, ie: 
		 * next time the matrix is queried it is recomputed
		 */
		void setDirty()							{ mDirty = true; }

		/**
		 * Sets the object to look at
		 */
		//ObjectLinkAttribute lookAt				{ this, "lookAt", RTTI_OF(RenderableComponent) };

	private:
		mutable glm::mat4x4		mProjectionMatrix;		// The composed projection matrix

		mutable bool			mDirty = true;			// If the projection matrix needs to be recalculated
		float					mAspectRatio = 1.0f;
		
		PerpCameraProperties		mProperties;
		TransformComponent*		mTransformComponent;
	};
}
