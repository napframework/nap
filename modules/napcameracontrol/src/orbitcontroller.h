/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <componentptr.h>
#include "perspcameracomponent.h"
#include <glm/glm.hpp>

namespace nap
{
	class OrbitControllerInstance;
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
	 * Resource part of the orbit controller.
	 * The controller is a perspective camera controller that rotates the camera around a target point.
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target.
	 * Requires a nap::PointerInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI OrbitController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrbitController, OrbitControllerInstance)
	public:
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float		mMovementSpeed = 0.5f;		///< Property: "MovementSpeed" The speed with which to move
		float		mRotateSpeed = 0.005f;		///< Property: "RotateSpeed" The speed with which to rotate
		glm::vec3	mLookAtPos;					///< Property: "LookAtPosition" The world space position to look at
		 
		ComponentPtr<PerspCameraComponent>	mPerspCameraComponent;	///< Property: "PerspCameraComponent" Link to perspective camera that we are controlling
	};


	/**
	 * Instance part of the orbit controller.
	 * The controller is a perspective camera controller that rotates the camera around a target point.
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target.
	 * Requires a nap::PointerInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI OrbitControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		OrbitControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Enable responding to input for this controller. Set translate and look-at position.
		 * @param cameraPos Worldspace camera position to set.
		 * @param lookAtPos Worldspace position to target.
		 */
		void enable(const glm::vec3& cameraPos, const glm::vec3& lookAtPos);

		/**
		 * Enable this controller using the given look-at position.
		 * @param lookAtPos position to look at.
		 */
		void enable(const glm::vec3& lookAtPos);

		/**
		 * Disable responding to input for this controller.
		 */
		void disable() { mEnabled = false; }

		/**
		 * @return the current position the camera controlled by the orbit controller looks at. 
		 */
		const glm::vec3 getLookAtPos() const				{ return mLookAtPos; }

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

	private:
		// Camera mode
		enum class EMode
		{
			Idle,			///< No action
			Rotating,		///< Rotate around target
			Zooming			///< Zoom into/out of target
		};

		ComponentInstancePtr<PerspCameraComponent>		mPerspCameraComponent = { this, &OrbitController::mPerspCameraComponent };
		TransformComponentInstance*						mTransformComponent = nullptr;		// The transform component used to move the entity
		EMode											mMode = EMode::Idle;				// Camera mode
		glm::vec3										mLookAtPos;							// Target position to orbit around
		bool											mEnabled = false;					// Enables responding to input
	};

}
