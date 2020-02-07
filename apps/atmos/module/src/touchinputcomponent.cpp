#include "touchinputcomponent.h"

// External Includes
#include <entity.h>

// nap::touchinputcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::TouchInputComponent)
	RTTI_PROPERTY("Parameter",	&nap::TouchInputComponent::mParameter,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",		&nap::TouchInputComponent::mSpeed,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::touchinputcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TouchInputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void TouchInputComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(PointerInputComponent));
	}


	bool TouchInputComponentInstance::init(utility::ErrorState& errorState)
	{
		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "%s: missing PointerInputComponent", mID.c_str()))
			return false;

		// Copy members
		TouchInputComponent* resource = getComponent<TouchInputComponent>();
		mParameter = resource->mParameter.get();
		mSpeed = resource->mSpeed;

		// Connect pointer move slots
		pointer_component->pressed.connect(std::bind(&TouchInputComponentInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&TouchInputComponentInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&TouchInputComponentInstance::onMouseUp, this, std::placeholders::_1));


		return true;
	}


	void TouchInputComponentInstance::update(double deltaTime)
	{

	}


	void TouchInputComponentInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;
		mPressed = true;
	}


	void TouchInputComponentInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (!mEnabled || !mPressed)
			return;

		// Get parameter value and add new pointer value
		glm::vec2 param_value = mParameter->mValue;
		param_value += glm::vec2(pointerMoveEvent.mRelY, pointerMoveEvent.mRelX) * mSpeed;
		mParameter->setValue(param_value);
	}


	void TouchInputComponentInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		if (!mEnabled)
			return;
		mPressed = false;
	}

}