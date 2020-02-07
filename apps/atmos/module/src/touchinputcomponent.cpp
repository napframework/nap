#include "touchinputcomponent.h"

// External Includes
#include <entity.h>
#include <glm/glm.hpp>

// nap::touchinputcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::TouchInputComponent)
	RTTI_PROPERTY("Enabled",	&nap::TouchInputComponent::mEnabled,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Parameter",	&nap::TouchInputComponent::mParameter,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",		&nap::TouchInputComponent::mSpeed,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SmoothTime",	&nap::TouchInputComponent::mSmoothTime,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Center",		&nap::TouchInputComponent::mCenterPoint,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Bounds",		&nap::TouchInputComponent::mBounds,			nap::rtti::EPropertyMetaData::Required)
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
		mSpeed = resource->mSpeed.get();
		mSmoothTime = resource->mSmoothTime.get();
		mEnabled = resource->mEnabled.get();
		mCenterPoint = resource->mCenterPoint.get();
		mBounds = resource->mBounds.get();

		// Copy default target value
		mSmoother.setValue(mCenterPoint->mValue);

		// Connect pointer move slots
		pointer_component->pressed.connect(std::bind(&TouchInputComponentInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&TouchInputComponentInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&TouchInputComponentInstance::onMouseUp, this, std::placeholders::_1));

		return true;
	}


	void TouchInputComponentInstance::update(double deltaTime)
	{
		// Skip when disabled
		if (!mEnabled->mValue)
			return;

		// Update smoothing time
		mSmoother.mSmoothTime = mSmoothTime->mValue;

		// TODO: Reset target when parameter / preset is loaded.
		glm::vec2 new_target = mCenterPoint->mValue + mTarget;
		glm::vec2 updated_value = mSmoother.update(new_target, deltaTime);
		mParameter->setValue(updated_value);
	}


	void TouchInputComponentInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled->mValue)
			return;
		mPressed = true;
	}


	void TouchInputComponentInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (!mEnabled->mValue || !mPressed)
			return;

		// Get parameter value and add new pointer value
		// Clamp because we want to keep in range here
		mTarget += glm::vec2(pointerMoveEvent.mRelX, pointerMoveEvent.mRelY) * mSpeed->mValue;
		mTarget.x = math::clamp<float>(mTarget.x, mBounds->mValue.x, mBounds->mValue.y);
		mTarget.y = math::clamp<float>(mTarget.y, mBounds->mValue.x, mBounds->mValue.y);
	}


	void TouchInputComponentInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		if (!mEnabled->mValue)
			return;
		mPressed = false;
	}

}