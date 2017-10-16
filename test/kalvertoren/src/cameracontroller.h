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

	enum class ECameraMode : uint8_t
	{
		None						= 0x00,

		FirstPerson					= 0x01,
		Orbit						= 0x02,
		OrthographicTop				= 0x04,
		OrthographicBottom			= 0x08,
		OrthographicLeft			= 0x10,
		OrthographicRight 			= 0x20,
		OrthographicFront			= 0x40,
		OrthographicBack			= 0x80,

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
	* Resource for the OrbitController
	*/
	class CameraController : public Component
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

		nap::EntityPtr	mLookAtTarget;
	};

	/**
	*
	*/
	class CameraControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		CameraControllerInstance(EntityInstance& entity, Component& resource);

		/**
		* Initialize this ComponentInstance
		*/
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		CameraComponentInstance& getCameraComponent();

	private:
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);
		void onKeyPress(const KeyPressEvent& keyReleaseEvent);

	private:
		void storeLastPerspTransform();
		void switchMode(ECameraMode targetMode);

	private:
		ECameraMode							mMode = ECameraMode::FirstPerson;
		OrbitControllerInstance*			mOrbitComponent = nullptr;
		FirstPersonControllerInstance*		mFirstPersonComponent = nullptr;
		OrthoControllerInstance*			mOrthoComponent = nullptr;
		glm::vec3							mLastPerspPos;
		glm::quat							mLastPerspRotate;
	};

}