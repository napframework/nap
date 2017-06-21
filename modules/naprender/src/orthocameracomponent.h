#pragma once

#include "cameracomponent.h"

namespace nap
{
	class OrthoCameraComponent;
	class TransformComponent;

	struct OrthoCameraProperties
	{
		float mNearClippingPlane = 1.0f;
		float mFarClippingPlane = 1000.0f;
	};
	
	class OrthoCameraComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) { components.push_back(RTTI_OF(TransformComponent)); }
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(OrthoCameraComponent); }

	public:
		OrthoCameraProperties mProperties;
	};

	/**
	 * An orthographic camera.
	 */
	class OrthoCameraComponent : public CameraComponent
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		OrthoCameraComponent(EntityInstance& entity);

		/**
		 * 
		 */
		virtual bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState);

		/**
		* Convenience method to specify lens aspect ratio, defined as width / height
		* @param width, arbitrary width, most often the resolution of the canvas
		* @param height, arbitrary height, most often the resolution of the canvas
		*/
		virtual void setRenderTargetSize(glm::ivec2 size) override;

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
		 * Sets this camera to be dirty, ie: 
		 * next time the matrix is queried it is recomputed
		 */
		void setDirty()							{ mDirty = true; }

	private:
		mutable glm::mat4x4		mProjectionMatrix;		// The composed projection matrix

		mutable bool			mDirty = true;			// If the projection matrix needs to be recalculated
		glm::ivec2				mRenderTargetSize;		// The size of the rendertarget we're rendering to
		
		OrthoCameraProperties	mProperties;
		TransformComponent*		mTransformComponent;
	};
}
