#pragma once

#include "nap/component.h"
#include "nap/entityptr.h"
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
	enum class NAPAPI ECameraMode : uint8_t
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
	 * CameraController drives the FirstPersonController, OrbitController and OrthoController. It takes care of switching
	 * between the camera controllers and it holds pointer to an entity to look at, for use in the OrbitController and the OrthoController.
	 */
	class NAPAPI CameraController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CameraController, CameraControllerInstance)
	public:
		/**
		* Get the types of components on which this component depends
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(OrbitController));
			components.push_back(RTTI_OF(FirstPersonController));
		}

		nap::EntityPtr	mLookAtTarget;		///< Object to look at, for orbit and ortho controller
	};


	/**
	 * ComponentInstance of CameraController. Use this class to retrieve the active camera component.
	 */
	class NAPAPI CameraControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CameraControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * @return either a perspective camera component or an orthographic camera component, depending on which 
		 * one is currently active.
		 */
		CameraComponentInstance& getCameraComponent();

	private:
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);
		void onKeyPress(const KeyPressEvent& keyReleaseEvent);

	private:
		void storeLastPerspTransform();
		void switchMode(ECameraMode targetMode);

	private:
		ECameraMode							mMode = ECameraMode::FirstPerson;		///< Camera mode
		OrbitControllerInstance*			mOrbitComponent = nullptr;				///< Orbit Controller
		FirstPersonControllerInstance*		mFirstPersonComponent = nullptr;		///< FPS Controller
		OrthoControllerInstance*			mOrthoComponent = nullptr;				///< Ortho controller
		glm::vec3							mLastPerspPos;							///< Last perspective camera position
		glm::quat							mLastPerspRotate;						///< Last perspective camera rotation
	};

}