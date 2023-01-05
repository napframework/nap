/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	 * Orthographic camera space operation mode
	 */
	enum EOrthoCameraMode
	{
		PixelSpace = 0,			///< Planes are scaled automatically to pixel coordinates. Near/Far is retrieved from properties.
		CorrectAspectRatio,		///< User provides all planes, but height is recalculated for correct aspect ratio
		Custom					///< All planes are retrieved from properties
	};

	
	/**
	 * Orthographic camera properties
	 */
	struct NAPAPI OrthoCameraProperties
	{
		EOrthoCameraMode mMode = PixelSpace;			///< Property: 'Mode' defaults to pixel space
		float mNearClippingPlane = 1.0f;				///< Property: 'NearClippingPlane' camera near clipping plane
		float mFarClippingPlane = 1000.0f;				///< Property: 'FarClippingPlane' camera far clipping plane
		float mLeftPlane = 0.0f;						///< Property: 'LeftPlane' used when mode is CorrectAspectRatio or Custom
		float mRightPlane = 100.0f;						///< Property: 'RightPlane' used when mode is CorrectAspectRatio or Custom
		float mTopPlane = 100.0f;						///< Property: 'TopPlane' used when mode is CorrectAspectRatio or Custom 
		float mBottomPlane = 0.0f;						///< Property: 'TopPlane' used when mode is CorrectAspectRatio or Custom
		math::Rect mClipRect = { {0.0f, 0.0f}, {1.0f, 1.0f} };	///< Property: 'ClipRect' normalized rectangle used to clip/crop the camera view
	};
	
	/**
	 * Orthographic camera resource.
	 */
	class NAPAPI OrthoCameraComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrthoCameraComponent, OrthoCameraComponentInstance)

		/**
		 * This camera depends on a transform to calculate the view matrix.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

	public:
		OrthoCameraProperties mProperties;		///< Property:'Properties' the camera settings
	};


	//////////////////////////////////////////////////////////////////////////
	// OrthoCameraComponentInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * An orthographic camera. The space that the camera is operating depends on the mode that is set. 
	 * By default this camera operates in pixel space. Use setMode to change the space that the camera is operating in.
	 * The transform to calculate the view matrix is retrieved from the transform component.
	 */
	class NAPAPI OrthoCameraComponentInstance : public CameraComponentInstance
	{
		RTTI_ENABLE(CameraComponentInstance)
	public:

		// Default constructor
		OrthoCameraComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this component from its resource
		 * @param errorState contains the error if initialization fails.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * This implementation extracts the size in pixels of the render target to make sure that the orthographic
		 * camera acts in pixel coordinates.
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
		* @return The planes for this orthographic camera.
		*/
		const OrthoCameraProperties& getProperties() { return mProperties; }

		/**
		* Sets the orthographic camera properties.
		* @param properties The new camera properties
		*/
		void setProperties(const OrthoCameraProperties& properties);

		/**
		* Sets the mode (the physical space) in which the orthographic camera is operating.
		* @param mode The view mode to operate in.
		*/
		void setMode(EOrthoCameraMode mode);

		/**
		* Sets the normalized clip rectangle used to clip/crop the camera view
		* @param clipRect The clip rectangle.
		*/
		void setClipRect(const math::Rect& clipRect);


		/**
		* Restores the clip rectangle to a unit rectangle, disabling clipping
		*/
		void restoreClipRect();

		/**
		 * Returns the matrix that is used to transform a 3d scene in to a 2d projection by the renderer.
	     * Vulkan uses a coordinate system where (-1, -1) is in the top left quadrant, instead of the bottom left quadrant.
		 * Use this matrix, instead of the one returned by getProjectionMatrix(), when an ortographic projection matrix is required as shader input.
		 * For all regular (CPU) related orthographic calculations, use getProjectionMatrix().
		 * @return the projection matrix used by the renderer
		 */
		virtual const glm::mat4& getRenderProjectionMatrix() const override;

		/**
		 * Creates a Vulkan compatible orthographic projection matrix, use this matrix instead of glm::ortho when interfacing with the renderer.
		 * Use this matrix, instead of glm::ortho(), when drawing geometry and an ortographic projection matrix is required as shader input.
		 * For all regular (CPU) related orthographic calculations, use glm::ortho().
		 * @return Vulkan compatible orthographic matrix.
		 */
		static glm::mat4 createRenderProjectionMatrix(float left, float right, float bottom, float top, float zNear, float zFar);

		/**
		 * Creates a Vulkan compatible orthographic projection matrix without having to specify near and far coordinates.
		 * Use this matrix, instead of glm::ortho(), when drawing geometry and an ortographic projection matrix is required as shader input.
		 * For all regular (CPU) related orthographic calculations, use glm::ortho().
		 * @return Vulkan compatible orthographic matrix.
		 */
		static glm::mat4 createRenderProjectionMatrix(float left, float right, float bottom, float top);

	private:

		/**
		 * The projection matrix of this camera is recomputed when requested
		 */
		void setDirty()					{ mDirty = true; }
			
		/**
		 * Updates all projection matrices when dirty
		 */
		void updateProjectionMatrices() const;

	private:
		mutable glm::mat4x4				mProjectionMatrix;						// The composed projection matrix
		mutable glm::mat4x4				mRenderProjectionMatrix;				// The composed projection matrix used by the renderer
		mutable bool					mDirty = true;							// If the projection matrix needs to be recalculated
		OrthoCameraProperties			mProperties;							// These properties are copied from the resource to the instance. When these are changed, only the instance is affected
		TransformComponentInstance*		mTransformComponent;					// Cached transform component
	};
}
