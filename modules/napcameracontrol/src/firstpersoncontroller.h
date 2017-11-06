#pragma once

#include "nap/component.h"
#include "nap/componentptr.h"
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
	 * Resource for the FirstPersonController
	 */
	class NAPAPI FirstPersonController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FirstPersonController, FirstPersonControllerInstance)
	public:
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
			components.push_back(RTTI_OF(KeyInputComponent));
		}

		float mMovementSpeed	= 3.0f;		// The speed with which to move
		float mRotateSpeed		= 1.0f;		// The speed with which to rotate

		ComponentPtr<nap::PerspCameraComponent>	mPerspCameraComponent;		// Camera that we're controlling
	};


	/**
	 * The FirstPersonController is a component that implements first-person movement for the entity it is attached to.
	 * It uses the TransformComponent to move the entity and the InputComponent to receive input
	 *
	 * Hold left mouse to activate. WASD to move, QE to move up and down, mouse to rotate.
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
	};

}