#pragma once

#include "nap/component.h"
#include "orbitcontroller.h"
#include "firstpersoncontroller.h"

namespace nap
{
	class CameraControllerInstance;
	class KeyReleaseEvent;

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

	private:
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);
		void onKeyPress(const KeyPressEvent& keyReleaseEvent);

	private:
		enum class EMode
		{
			FirstPerson,
			Orbit
		};

		EMode							mMode = EMode::FirstPerson;
		OrbitControllerInstance*		mOrbitComponent = nullptr;
		FirstPersonControllerInstance*	mFirstPersonComponent = nullptr;
	};

}