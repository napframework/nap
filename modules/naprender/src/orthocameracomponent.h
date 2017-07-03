#pragma once

#include "cameracomponent.h"

namespace nap
{
	class OrthoCameraComponent;
	class TransformComponent;
	class TransformComponentResource;

	
	/**
	 * Properties for orthographic camera. Used in both resource and instance.
	 */
	struct OrthoCameraProperties
	{
		float mNearClippingPlane = 1.0f;
		float mFarClippingPlane = 1000.0f;
	};
	
	/**
	 * Orthographic camera resource, hold json properties for the camera.
	 */
	class OrthoCameraComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)

		/**
		 * Camera is dependent on the transform component for calculating the view matrix.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) { components.push_back(RTTI_OF(TransformComponentResource)); }

		/**
		 * Returns instance type to create for this ComponentResource.
		 */
		virtual const rtti::TypeInfo getInstanceType() const { return RTTI_OF(OrthoCameraComponent); }

	public:
		OrthoCameraProperties mProperties;		// Properties of the camera
	};

	/**
	 * An orthographic camera. The space that the camera is operating in is in pixel coordinates.
	 * Be sure to call setRenderTargetSize so that the camera's space is updated correctly.
	 * The transform to calculate the view matrix is retrieved from the transform component.
	 */
	class OrthoCameraComponent : public CameraComponent
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		OrthoCameraComponent(EntityInstance& entity);

		/**
		* Initialize this component from its resource
		*
		* @param resource The resource we're being instantiated from
		* @param entityCreationParams Parameters required to create new entity instances during init
		* @param errorState The error object
		*/
		virtual bool init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

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

	private:

		/**
		* Sets this camera to be dirty, ie:
		* next time the matrix is queried it is recomputed
		*/
		void setDirty() { mDirty = true; }

	private:

		mutable glm::mat4x4		mProjectionMatrix;		// The composed projection matrix

		mutable bool			mDirty = true;			// If the projection matrix needs to be recalculated
		glm::ivec2				mRenderTargetSize;		// The size of the rendertarget we're rendering to
		
		OrthoCameraProperties	mProperties;			// These properties are copied from the resource to the instance. When these are changed, only the instance is affected
		TransformComponent*		mTransformComponent;	// Cached transform component
	};
}
