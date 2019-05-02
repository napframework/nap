#pragma once

#include <component.h>
#include <componentptr.h>
#include <orbitcontroller.h>
#include <firstpersoncontroller.h>
#include <numeric>
#include <parametersimple.h>
#include <parameternumeric.h>
#include <transformcomponent.h>

namespace nap
{
	class ControlSelectComponentInstance;

	enum class EControlMethod : uint8_t
	{
		Orbit = 0,
		FirstPerson = 1
	};

	/**
	 *	controlselectcomponent
	 */
	class NAPAPI ControlSelectComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ControlSelectComponent, ControlSelectComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		EControlMethod							mControlMethod = EControlMethod::Orbit;
		ComponentPtr<OrbitController>			mOrbitController;							///< Property: "OrbitController"
		ComponentPtr<FirstPersonController>		mFirstPersonController;						///< Property: "FirstPersonController"
		ResourcePtr<ParameterQuat>				mCameraRotation;							///< Property: "CameraRotation"
		ResourcePtr<ParameterVec3>				mCameraTranslation;							///< Property: "CameraTranslation"
	};


	/**
	 * controlselectcomponentInstance	
	 */
	class NAPAPI ControlSelectComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ControlSelectComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize controlselectcomponentInstance based on the controlselectcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the controlselectcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update controlselectcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Selects the currently active control method
		 */
		void selectControlMethod(EControlMethod method);

		/**
		 *	@return the currently selected control method
		 */
		EControlMethod getCurrentControlMethod() const { return mControlMethod; }

		ComponentInstancePtr<OrbitController> mOrbitController =	{ this, &ControlSelectComponent::mOrbitController };
		ComponentInstancePtr<FirstPersonController>	mFirstPersonController = { this, &ControlSelectComponent::mFirstPersonController };
		ParameterVec3* mCameraTranslation = nullptr;
		ParameterQuat* mCameraRotation = nullptr;
		EControlMethod mControlMethod = EControlMethod::Orbit;
		TransformComponentInstance* mCameraTransformComponent = nullptr;
	};
}
