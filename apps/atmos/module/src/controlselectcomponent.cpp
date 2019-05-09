#include "controlselectcomponent.h"

// External Includes
#include <entity.h>

// nap::controlselectcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ControlSelectComponent)
	RTTI_PROPERTY("OrbitController",		&nap::ControlSelectComponent::mOrbitController,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FirstPersonController",	&nap::ControlSelectComponent::mFirstPersonController,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PerspectiveCamera",		&nap::ControlSelectComponent::mCameraComponent,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraTranslation",		&nap::ControlSelectComponent::mCameraTranslation,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraRotation",			&nap::ControlSelectComponent::mCameraRotation,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraFOV",				&nap::ControlSelectComponent::mCameraFOV,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraMoveSpeed",		&nap::ControlSelectComponent::mCameraMovSpeed,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraRotateSpeed",		&nap::ControlSelectComponent::mCameraRotSpeed,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CameraControlMethod",	&nap::ControlSelectComponent::mCameraControlMethod,		nap::rtti::EPropertyMetaData::Required)
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
		mCameraTranslation		= getComponent<ControlSelectComponent>()->mCameraTranslation.get();
		mCameraRotation			= getComponent<ControlSelectComponent>()->mCameraRotation.get();
		mCameraRotSpeed			= getComponent<ControlSelectComponent>()->mCameraRotSpeed.get();
		mCameraMovSpeed			= getComponent<ControlSelectComponent>()->mCameraMovSpeed.get();
		mCameraFOV				= getComponent<ControlSelectComponent>()->mCameraFOV.get();
		mCameraControlMethod	= getComponent<ControlSelectComponent>()->mCameraControlMethod.get();

		// Store current min / max camera movement speed values
		mCamMaxMovSpeed = mFirstPersonController->getMovementSpeed();
		mCamMaxRotSpeed = mFirstPersonController->getRotationSpeed();

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

		// Update camera movement speed when value changes
		mCameraMovSpeed->valueChanged.connect([this](float newValue)
		{
			mFirstPersonController->setMovementSpeed(mCamMaxMovSpeed * newValue);
		});
		
		// Update camera rotation speed when value changes
		mCameraRotSpeed->valueChanged.connect([this](float newValue)
		{
			mFirstPersonController->setRotationSpeed(mCamMaxRotSpeed * newValue);
		});

		// Update camera FOV when value changes
		mCameraFOV->valueChanged.connect([this](float newValue)
		{
			mCamera->setFieldOfView(newValue);
		});

		// Update control method on param change
		mCameraControlMethod->valueChanged.connect([this](EControlMethod newValue)
		{
			this->selectControlMethod(newValue);
		});

		// Force control mode now
		selectControlMethod((EControlMethod)mCameraControlMethod->getValue());
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