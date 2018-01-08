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
	 * OrbitController is a camera controller for a perspective camera that rotates around a target point.
	 * 
	 * Left mouse button to rotate, right mouse button to zoom in and out on the target.
	 */
	class NAPAPI OrbitController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrbitController, OrbitControllerInstance)
	public:
		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
			components.push_back(RTTI_OF(KeyInputComponent));
		}

		float		mMovementSpeed = 0.5f;		///< The speed with which to move
		float		mRotateSpeed = 0.005f;		///< The speed with which to rotate
		glm::vec3	mLookAtPos;					///< The worldspace position to look at

		ComponentPtr<PerspCameraComponent>	mPerspCameraComponent;	///< Perspective camera that we are controlling
	};


	/**
	 * ComponentInstance for the OrbitController.
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
		 * Enable responding to input for this controller, and set translate and lookat.
		 * @param cameraPos Worldspace camera position to set.
		 * @param lookAtPos Worldspace position to target.
		 */
		void enable(const glm::vec3& cameraPos, const glm::vec3& lookAtPos);

		/**
		 * Enable responding to input for this controller.
		 */
		void enable(const glm::vec3& lookAtPos);

		/**
		 * Disable responding to input for this controller.
		 */
		void disable() { mEnabled = false; }

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
			Idle,
			Rotating,		// Rotate around target
			Zooming			// Zoom into/out of target
		};

		ComponentInstancePtr<PerspCameraComponent>		mPerspCameraComponent = { this, &OrbitController::mPerspCameraComponent };
		TransformComponentInstance*						mTransformComponent = nullptr;		// The transform component used to move the entity
		EMode											mMode = EMode::Idle;				// Camera mode
		glm::vec3										mLookAtPos;							// Target position to orbit around
		bool											mEnabled = false;					// Enables responding to input
	};

}