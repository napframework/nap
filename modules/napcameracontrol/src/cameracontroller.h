/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <entityptr.h>
#include "orbitcontroller.h"
#include "firstpersoncontroller.h"
#include "orthocontroller.h"

namespace nap
{
	class CameraControllerInstance;
	class KeyReleaseEvent;

	/**
	 * Mode that the CameraController is currently operating in. 
	 */
	enum class ECameraMode : uint8_t
	{
		None						= 0x00,

		FirstPerson					= 0x01,		// Perspective free camera
		Orbit						= 0x02,		// Perspective orbit camera
		OrthographicTop				= 0x04,		// Orthographic camera (top-view)
		OrthographicBottom			= 0x08,		// Orthographic camera (bottom-view)
		OrthographicLeft			= 0x10,		// Orthographic camera (left-view)
		OrthographicRight 			= 0x20,		// Orthographic camera (right-view)
		OrthographicFront			= 0x40,		// Orthographic camera (front-view)
		OrthographicBack			= 0x80,		// Orthographic camera (back-view)
		Perspective = FirstPerson | Orbit,
		Orthographic = OrthographicTop | OrthographicBottom | OrthographicLeft | OrthographicRight | OrthographicFront | OrthographicBack
	};

	inline ECameraMode operator&(ECameraMode a, ECameraMode b)
	{
		return static_cast<ECameraMode>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
	}
	inline ECameraMode operator|(ECameraMode a, ECameraMode b)
	{
		return static_cast<ECameraMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
	}


	/**
	 * Resource part of the camera controller.
	 * The camera controller allows for switching between various controllers, ie: 
	 * the nap::FirstPersonController, nap::OrbitController, and nap::OrthoController.
	 * It holds a pointer to an entity to look at when the orbit or ortho controller is selected.
	 */
	class NAPAPI CameraController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CameraController, CameraControllerInstance)
	public:
		/**
		 * Returns the controllers this component depends upon.
		 * @param components the various controllers this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.emplace_back(RTTI_OF(OrbitController));
			components.emplace_back(RTTI_OF(FirstPersonController));
			components.emplace_back(RTTI_OF(OrthoController));
		}

		nap::EntityPtr	mLookAtTarget;		///< Property: 'LookAtTarget' Object to look at, used by the orbit and ortho controller
	};


	/**
	 * Instance part of the camera controller.
	 * The camera controller allows for switching between various controllers, ie:
	 * the nap::FirstPersonController, nap::OrbitController, and nap::OrthoController.
	 * It holds a pointer to an entity to look at when the orbit or ortho controller is selected.
	 */
	class NAPAPI CameraControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CameraControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return either a perspective camera component or an orthographic camera component, depending on which 
		 * one is currently active.
		 */
		CameraComponentInstance& getCameraComponent();

	private:
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);
		void onKeyPress(const KeyPressEvent& keyReleaseEvent);
		void storeLastPerspTransform();
		void switchMode(ECameraMode targetMode);

	private:
		EntityInstancePtr					mLookAtTarget = { this, &CameraController::mLookAtTarget };
		ECameraMode							mMode = ECameraMode::FirstPerson;		///< Camera mode
		OrbitControllerInstance*			mOrbitComponent = nullptr;				///< Orbit Controller
		FirstPersonControllerInstance*		mFirstPersonComponent = nullptr;		///< FPS Controller
		OrthoControllerInstance*			mOrthoComponent = nullptr;				///< Ortho controller
		glm::vec3							mLastPerspPos;							///< Last perspective camera position
		glm::quat							mLastPerspRotate;						///< Last perspective camera rotation
	};

}