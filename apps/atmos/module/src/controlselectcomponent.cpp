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
		selectControlMethod(getComponent<ControlSelectComponent>()->mControlMethod);
		return true;
	}


	void ControlSelectComponentInstance::update(double deltaTime)
	{

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