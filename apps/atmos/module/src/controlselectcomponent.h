#pragma once

// Local Includes
#include "enumparameters.h"

// External Includes
#include <component.h>
#include <componentptr.h>
#include <orbitcontroller.h>
#include <firstpersoncontroller.h>
#include <numeric>
#include <parametersimple.h>
#include <parameternumeric.h>
#include <transformcomponent.h>
#include <parameterquat.h>
#include <parametervec.h>

// Local Includes
#include "followpathcontroller.h"

namespace nap
{
	class ControlSelectComponentInstance;

	/**
	 *	Resource parth. Allows for switching between the various camera control methods.
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

		ComponentPtr<OrbitController>			mOrbitController;							///< Property: "OrbitController"
		ComponentPtr<FirstPersonController>		mFirstPersonController;						///< Property: "FirstPersonController"
		ComponentPtr<FollowPathController>		mPathController;							///< Property: "PathController"
		ComponentPtr<PerspCameraComponent>		mCameraComponent;							///< Property: "PerspectiveCamera"
		ResourcePtr<ParameterQuat>				mCameraRotation;							///< Property: "CameraRotation"
		ResourcePtr<ParameterVec3>				mCameraTranslation;							///< Property: "CameraTranslation"
		ResourcePtr<ParameterFloat>				mFPSCameraMovSpeed = nullptr;
		ResourcePtr<ParameterFloat>				mFPSCameraRotSpeed = nullptr;
		ResourcePtr<ParameterFloat>				mCameraFOV = nullptr;
		ResourcePtr<ParameterFloat>				mPathCamPosition = nullptr;
		ResourcePtr<ParameterFloat>				mPathCamSpeed = nullptr;	
		ResourcePtr<ParameterVec3>				mPathCamOffset = nullptr;
		ResourcePtr<ParameterVec2>				mPathCamRotation = nullptr;
		ResourcePtr<ParameterControlMethod>		mCameraControlMethod = nullptr;
	};


	/**
	 *	Resource part. Allows for switching between the various camera control methods.
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

		// Component links
		ComponentInstancePtr<OrbitController> mOrbitController =				{ this, &ControlSelectComponent::mOrbitController };
		ComponentInstancePtr<FirstPersonController>	mFirstPersonController =	{ this, &ControlSelectComponent::mFirstPersonController };
		ComponentInstancePtr<FollowPathController> mPathController =			{ this, &ControlSelectComponent::mPathController };
		ComponentInstancePtr<PerspCameraComponent>	mCamera =					{ this, &ControlSelectComponent::mCameraComponent };

		// Found components
		TransformComponentInstance* mCameraTransformComponent = nullptr;

		// Current control method
		EControlMethod mControlMethod = EControlMethod::Orbit;

		// Pointers to parameters
		ParameterVec3*				mCameraTranslation = nullptr;
		ParameterQuat*				mCameraRotation = nullptr;
		ParameterFloat*				mFPSCameraMovSpeed = nullptr;
		ParameterFloat*				mFPSCameraRotSpeed = nullptr;
		ParameterFloat*				mCameraFOV = nullptr;
		ParameterControlMethod*		mCameraControlMethod = nullptr;
		ParameterFloat*				mPathCamPosition = nullptr;
		ParameterFloat*				mPathCamSpeed = nullptr;
		ParameterVec3*				mPathCamOffset = nullptr;
		ParameterVec2*				mPathCamRotation = nullptr;

		float						mCamMaxRotSpeed = 1.0f;
		float						mCamMaxMovSpeed = 1.0f;
	};
}
