/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <componentptr.h>
#include "perspcameracomponent.h"
#include <glm/gtx/quaternion.hpp>

namespace nap
{
	class FirstPersonControllerInstance;
	class KeyPressEvent;
	class KeyReleaseEvent;
	class PointerMoveEvent;
	class PointerPressEvent;
	class PointerReleaseEvent;
	class TransformComponentInstance;
	class TransformComponent;
	class KeyInputComponent;
	class KeyInputComponent;

	/**
	 * Resource part of the first person controller.
	 * Applies first-person movement to the (camera) entity it is attached to.
	 * Hold left mouse to activate. WASD to move, QE to move up and down, mouse to rotate.
	 * Requires a nap::KeyInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI FirstPersonController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FirstPersonController, FirstPersonControllerInstance)
	public:
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float mMovementSpeed	= 3.0f;		///< Property: "MovementSpeed" The speed with which to move
		float mRotateSpeed		= 1.0f;		///< Property: "RotateSpeed" The speed with which to rotate

		ComponentPtr<nap::PerspCameraComponent>	mPerspCameraComponent;		///< Property: "PerspCameraComponent" Camera that we're controlling
	};


	/**
	 * Instance part of the first person controller.
	 * The controller applies first-person movement to the (camera) entity it is attached to.
	 * Hold left mouse to activate. WASD to move, QE to move up and down, mouse to rotate.
	 * Requires a nap::KeyInputComponent and nap::TransformComponent to be present on the same entity.
	 */
	class NAPAPI FirstPersonControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FirstPersonControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Update this ComponentInstance
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Enable the component while setting the transform.
		 * @param translate Camera translation to set.
		 * @param rotate Camera rotation to set.
		 */
		void enable(const glm::vec3& translate, const glm::quat& rotate);

		/**
		 * Enable the component, keeping the current transform.
		 */
		void enable();

		/**
		 * Disables the component.
		 */
		void disable() { mEnabled = false; }

		/**
		 * @return The perspective camera component that we're controlling.
		 */
		CameraComponentInstance& getCameraComponent();

		/**
		 *	@return the camera movement speed
		 */
		float getMovementSpeed() const							{ return mMovementSpeed; }

		/**
		 * Sets the camera movement speed
		 * @param speed the new movement speed
		 */
		void setMovementSpeed(float speed)						{ mMovementSpeed = speed; }

		/**
		 *	@return current camera rotation speed
		 */
		float getRotationSpeed() const							{ return mRotateSpeed; }

		/**
		 * Sets the current camera rotation speed
		 * @param speed the new camera rotation speed
		 */
		void setRotationSpeed(float speed)						{ mRotateSpeed = speed; }

	private:
		/**
		 * Handler for key press events
		 */
		void onKeyPress(const KeyPressEvent& keyPressEvent);

		/**
		 * Handler for key release events
		 */
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);

		/**
		 * Handler for mouse down events
		 */
		void onMouseDown(const PointerPressEvent& pointerPressEvent);

		/**
		 * Handler for mouse up events
		 */
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);

		/**
		 * Handler for mouse move events
		 */
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);

	private:
		ComponentInstancePtr<PerspCameraComponent> mPerspCameraComponent = { this, &FirstPersonController::mPerspCameraComponent };

		TransformComponentInstance*		mTransformComponent = nullptr;		// The transform component used to move the entity
		bool							mMoveForward		= false;		// Whether we're moving forward
		bool							mMoveBackward		= false;		// Whether we're moving backwards
		bool							mMoveLeft			= false;		// Whether we're moving left
		bool							mMoveRight			= false;		// Whether we're moving right
		bool							mMoveUp				= false;		// Whether we're moving up
		bool							mMoveDown			= false;		// Whether we're moving down
		bool							mEnabled			= true;			// Set if enable() is called
		bool							mMoving				= false;		// Set if left mouse button is held
		float							mMovementSpeed		= 1.0f;			// Movement Speed
		float							mRotateSpeed		= 1.0f;			// Rotate Speed
	};

}
