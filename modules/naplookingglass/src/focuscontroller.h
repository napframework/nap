/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "quiltcameracomponent.h"

#include <component.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <glm/glm.hpp>

namespace nap
{
	class FocusControllerInstance;
	class KeyPressEvent;
	class KeyReleaseEvent;
	class PointerPressEvent;
	class PointerMoveEvent;
	class PointerReleaseEvent;
	class TransformComponentInstance;
	class TransformComponent;
	class KeyInputComponent;
	class KeyInputComponent;

	/**
	 * Resource part of the focus controller.
	 * This is a perspective camera controller that rotates the camera around a target point - for quilt cameras.
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target.
	 * Requires a nap::PointerInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI FocusController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FocusController, FocusControllerInstance)
	public:
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float		mZoomSpeed = 0.5f;			///< Property: 'ZoomSpeed' the speed with which to zoom.
		float		mMovementSpeed = 0.5f;		///< Property: 'MovementSpeed' the speed with which to move.
		float		mRotateSpeed = 0.005f;		///< Property: 'RotateSpeed' the speed with which to rotate.
		float		mMinZoomDistance = 1.0f;	///< Property: 'MinZoomDistance' the distance at which to limit the camera zoom.
		bool		mAutoFocus = false;			///< Property: 'AutoFocus' whether to keep the target in focus at all times.

		ComponentPtr<TransformComponent>	mLookAtTargetComponent;	///< Property: 'mLookAtTarget' transform of the world space position to look at	 .
	};


	/**
	 * Instance part of the orbit controller.
	 * The controller is a perspective camera controller that rotates the camera around a target point.
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target, middle button to
	 * move the camera along its current direction vector to the optimal focus distance from the target transform.
	 * Requires a nap::PointerInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI FocusControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FocusControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Update this component
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Focus the camera on the focal position
		 */
		void focus();

		/**
		 * Enable responding to input for this controller. Set translate and look-at position.
		 */
		void enable() { mEnabled = true; }

		/**
		 * Disable responding to input for this controller.
		 */
		void disable() { mEnabled = false; }

		/**
		 * 
		 */
		void setAutoFocus(bool enable) { mAutoFocus = enable; focus(); }

		/**
		 *
		 */
		bool isAutoFocusEnabled() const { return mAutoFocus; }

		/**
		 * @return the focal position the camera. 
		 */
		glm::vec3 getFocalPosition() const;

		/**
		 * Sets the current lookat target transform 
		 * @param transform the transform 
		 */
		void setLookAtTarget(TransformComponentInstance& transform);

		/**
		 * @return the current lookat target transform
		 */
		const TransformComponentInstance& getLookAtTarget() const { return *mCurrentLookAtTarget; }

		/**
		 * @return The perspective camera component that we are controlling.
		 */
		CameraComponentInstance& getCameraComponent();

	private:
		/**
		 * Handler for mouse down events
		 */
		void onMouseDown(const PointerPressEvent& pointerPressEvent);

		/**
		 * Handler for mouse move events
		 */
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);

		/**
		 * Handler for mouse up events
		 */
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);

		/**
		 * Handler for key press events
		 */
		void onKeyPress(const KeyPressEvent& keyPressEvent);

		/**
		 * Handler for key release events
		 */
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);

	private:
		// Camera mode
		enum class EMode
		{
			Idle,			///< No action
			Rotating,		///< Rotate around target
			Zooming,		///< Zoom into/out of target
			Focus			///< Focus distance
		};

		ComponentInstancePtr<TransformComponent>	mLookAtTargetComponent = { this, &FocusController::mLookAtTargetComponent };
		TransformComponentInstance*					mCurrentLookAtTarget = nullptr;

		QuiltCameraComponentInstance*				mQuiltCameraComponent = nullptr;	// The camera component to control
		TransformComponentInstance*					mTransformComponent = nullptr;		// The transform component used to move the entity
		EMode										mMode = EMode::Idle;				// Camera mode
		bool										mEnabled = false;					// Enables responding to input

		bool										mAutoFocus = false;					// Enables autofocus

		bool										mMoveForward = false;				// Whether we're moving forward
		bool										mMoveBackward = false;				// Whether we're moving backwards
		bool										mMoveLeft = false;					// Whether we're moving left
		bool										mMoveRight = false;					// Whether we're moving right
		bool										mMoveUp = false;					// Whether we're moving up
		bool										mMoveDown = false;					// Whether we're moving down
	};

}
