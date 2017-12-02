#pragma once

// Local Includes
#include "cameracomponent.h"

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	class OrthoCameraComponentInstance;
	class TransformComponentInstance;
	class TransformComponent;

	
	/**
	 * Properties for orthographic camera. Used in both resource and instance.
	 */
	struct NAPAPI OrthoCameraProperties
	{
		float mLeftPlane = 0.0f;
		float mRightPlane = 100.0f;
		float mTopPlane = 100.0f;
		float mBottomPlane = 0.0f;
		float mNearClippingPlane = 1.0f;
		float mFarClippingPlane = 1000.0f;
	};
	
	/**
	 * Orthographic camera resource, hold json properties for the camera.
	 */
	class NAPAPI OrthoCameraComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrthoCameraComponent, OrthoCameraComponentInstance)

		/**
		 * Camera is dependent on the transform component for calculating the view matrix.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override { components.push_back(RTTI_OF(TransformComponent)); }

	public:
		OrthoCameraProperties mProperties;		// Properties of the camera
	};

	/**
	 * An orthographic camera. The space that the camera is operating depends on the mode that is set. By default, it is operating in pixel space.
	 * Use setMode to change the space that the camera is operating in.
	 * The transform to calculate the view matrix is retrieved from the transform component.
	 */
	class NAPAPI OrthoCameraComponentInstance : public CameraComponentInstance
	{
		RTTI_ENABLE(CameraComponentInstance)
	public:

		enum EMode
		{
			PixelSpace,				// left/right/top/bottom planes are scaled automatically to pixel coordinates. Near/far is retrieved from properties.
			CorrectAspectRatio,		// User provides all planes, but height is recalculated for correct aspect ratio
			Custom					// All planes are retrieved from properties
		};

		// Default constructor
		OrthoCameraComponentInstance(EntityInstance& entity, Component& resource);

		/**
		* Initialize this component from its resource
		*
		* @param resource The resource we're being instantiated from
		* @param entityCreationParams Parameters required to create new entity instances during init
		* @param errorState The error object
		*/
		virtual bool init(utility::ErrorState& errorState) override;

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
		* @return The planes for this orthographic camera.
		*/
		const OrthoCameraProperties& getProperties() { return mProperties; }

		/**
		* Sets the orthographic camera planes.
		* @param properties The camera planes to set.
		*/
		void setProperties(const OrthoCameraProperties& properties);

		/**
		* Sets the mode (the physical space) in which the orthographic camera is operating.
		* @param mode The mode to set.
		*/
		void setMode(EMode mode);

	private:

		/**
		* Sets this camera to be dirty, ie:
		* next time the matrix is queried it is recomputed
		*/
		void setDirty() { mDirty = true; }

	private:
		EMode							mMode = EMode::PixelSpace;	// By default we map to pixel space
		mutable glm::mat4x4				mProjectionMatrix;			// The composed projection matrix
		mutable bool					mDirty = true;				// If the projection matrix needs to be recalculated
		OrthoCameraProperties			mProperties;				// These properties are copied from the resource to the instance. When these are changed, only the instance is affected
		TransformComponentInstance*		mTransformComponent;		// Cached transform component
	};
}
