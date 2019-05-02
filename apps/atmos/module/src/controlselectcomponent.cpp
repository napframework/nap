#include "controlselectcomponent.h"

// External Includes
#include <entity.h>

RTTI_BEGIN_ENUM(nap::EControlMethod)
	RTTI_ENUM_VALUE(nap::EControlMethod::Orbit,			"Orbit"),
	RTTI_ENUM_VALUE(nap::EControlMethod::FirstPerson,	"FirstPerson")
RTTI_END_ENUM

// nap::controlselectcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ControlSelectComponent)
	RTTI_PROPERTY("OrbitController",		&nap::ControlSelectComponent::mOrbitController,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FirstPersonController",	&nap::ControlSelectComponent::mFirstPersonController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ControlMethod",			&nap::ControlSelectComponent::mControlMethod,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraTranslation",		&nap::ControlSelectComponent::mCameraTranslation,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraRotation",			&nap::ControlSelectComponent::mCameraRotation,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::controlselectcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ControlSelectComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ControlSelectComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ControlSelectComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get pointers to the camera parameters
		mCameraTranslation = getComponent<ControlSelectComponent>()->mCameraTranslation.get();
		mCameraRotation  = getComponent<ControlSelectComponent>()->mCameraRotation.get();

		// Get transform that is manipulated using the controllers
		mCameraTransformComponent = mOrbitController->getEntityInstance()->findComponent<nap::TransformComponentInstance>();
		if (!errorState.check(mCameraTransformComponent != nullptr, "unable to locate camera transform component"))
			return false;
		
		// Update position when camera location changes
		mCameraTranslation->valueChanged.connect([this](glm::vec3 newValue)
		{
			mCameraTransformComponent->setTranslate(newValue);
		});

		// Update rotation when camera location changes
		mCameraRotation->valueChanged.connect([this](glm::quat newValue)
		{
			mCameraTransformComponent->setRotate(newValue);
		});

		selectControlMethod(getComponent<ControlSelectComponent>()->mControlMethod);
		return true;
	}


	void ControlSelectComponentInstance::update(double deltaTime)
	{
		mCameraTranslation->setValue(mCameraTransformComponent->getTranslate());
		mCameraRotation->setValue(mCameraTransformComponent->getRotate());
	}


	void ControlSelectComponentInstance::selectControlMethod(EControlMethod method)
	{
		switch (method)
		{
		case EControlMethod::FirstPerson:
			mFirstPersonController->enable();
			mOrbitController->disable();
			break;
		case EControlMethod::Orbit:
			mFirstPersonController->disable();
			mOrbitController->enable(mOrbitController->getLookAtPos());
			break;
		default:
			assert(false);
		}

		mControlMethod = method;
	}

}