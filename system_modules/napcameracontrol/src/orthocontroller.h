/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <inputcomponent.h>
#include <orthocameracomponent.h>
#include <glm/glm.hpp>

namespace nap
{
	class OrthoControllerInstance;
	class PointerPressEvent;
	class PointerMoveEvent;
	class PointerReleaseEvent;
	class TransformComponentInstance;
	class TransformComponent;

	/**
	 * Resource part of the orthographic controller.
	 * Adds Orthographic camera control to the entity it is attached to.
	 * It uses the TransformComponent to move the entity and the InputComponent to receive input.
	 * These components are required to be present on the same entity.
	 * Hold left mouse button to pan, right mouse button to zoom.
	 */
	class NAPAPI OrthoController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrthoController, OrthoControllerInstance)
	public:
		/**
 		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float								mMovementSpeed = 1.0f;	///< Property: "MovementSpeed" The speed with which to move
		float								mZoomSpeed = 0.005f;	///< Property: "ZoomSpeed" The speed with which to zoom
		ComponentPtr<OrthoCameraComponent>	mOrthoCameraComponent;	///< Property: "OrthoCameraComponent" Camera that we're controlling
		bool								mEnable = true;			///< Property: "Enable" Whether camera control through this component is enabled
	};


	/**
	 * Instance part of the orthographic controller.
	 * Adds Orthographic camera control to the entity it is attached to.
	 * It uses the TransformComponent to move the entity and the InputComponent to receive input.
	 * These components are required to be present on the same entity.
	 * Hold left mouse button to pan, right mouse button to zoom.
	 */
	class NAPAPI OrthoControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		OrthoControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Enables orthographic control based on the given camera position and rotation values.
		 * @param cameraPos Camera translation to set.
		 * @param cameraRotate Camera rotation to set.
		 */
		void enable(const glm::vec3& cameraPos, const glm::quat& cameraRotate);

		/**
		 * Disables controlling of the camera.
		 */
		void disable() { mEnabled = false; }

		/**
		 * @return The orthographic camera component that we're controlling.
		 */
		CameraComponentInstance& getCameraComponent();

	private:

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

		/**
		 * Updates camera planes based on pan and zoom
		 */
		void updateCameraProperties();

	private:
		enum EMode
		{
			None,
			Pan,		// Currently panning
			Zoom		// Currently zooming
		};

		ComponentInstancePtr<OrthoCameraComponent>		mOrthoCameraComponent = { this, &OrthoController::mOrthoCameraComponent };

		TransformComponentInstance*				mTransformComponent = nullptr;		// The transform component used to move the entity
		bool									mEnabled = true;					// Set if enabled for input
		float									mCameraScale = 1.0f;				// Current scale, selects the width of the space you can see
		float									mCameraScaleAtClick = 0.0f;			// Scale that was set when clicking with the mouse button
		EMode									mMode = EMode::None;				// Pan/Zoom mode
		glm::vec2								mMousePosAtClick;					// Mouse position that was set when clicking with the mouse button
		glm::vec2								mMousePosNow;						// Mouse position
		glm::vec3								mTranslateAtClick;					// Camera translation that was set when clicking with the mouse button
	};

}
