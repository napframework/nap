#include "osclaserinputhandler.h"

#include <nap/entity.h>
#include <polyline.h>
#include <utility/stringutils.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::OSCLaserInputHandler)
	RTTI_PROPERTY("SelectionComponentOne", &nap::OSCLaserInputHandler::mSelectionComponentOne, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SelectionComponentTwo", &nap::OSCLaserInputHandler::mSelectionComponentTwo, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LaserOutputComponent",  &nap::OSCLaserInputHandler::mLaserOutputComponent,  nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCLaserInputHandlerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	OSCLaserInputHandlerInstance::~OSCLaserInputHandlerInstance()
	{
		mInputComponent->messageReceived.disconnect(mMessageReceivedSlot);
	}


	bool OSCLaserInputHandlerInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mRotateComponent = getEntityInstance()->findComponent<nap::RotateComponentInstance>();
		if (!errorState.check(mRotateComponent != nullptr, "missing rotate component"))
			return false;

		mInputComponent = getEntityInstance()->findComponent<nap::OSCInputComponentInstance>();
		if (!errorState.check(mInputComponent != nullptr, "missing osc input component"))
			return false;

		mBlendComponent = getEntityInstance()->findComponent<nap::LineBlendComponentInstance>();
		if (!errorState.check(mBlendComponent != nullptr, "missing line blend component"))
			return false;

		mTransformComponent = getEntityInstance()->findComponent<nap::TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "missing transform component"))
			return false;
		mInitialScale = mTransformComponent->getUniformScale();

		mColorComponent = getEntityInstance()->findComponent<nap::LineColorComponentInstance>();
		if (!errorState.check(mColorComponent != nullptr, "missing color component"))
			return false;

		mSelectorOne = getComponent<OSCLaserInputHandler>()->mSelectionComponentOne.get();
		mSelectorTwo = getComponent<OSCLaserInputHandler>()->mSelectionComponentTwo.get();
		mLaserOutput = getComponent<OSCLaserInputHandler>()->mLaserOutputComponent.get();

		mInputComponent->messageReceived.connect(mMessageReceivedSlot);

		return true;
	}


	void OSCLaserInputHandlerInstance::handleMessageReceived(const nap::OSCEvent& oscEvent)
	{
		if (utility::gStartsWith(oscEvent.getAddress(), "/color"))
		{
			// Get values
			std::vector<std::string> out_values;
			utility::gSplitString(oscEvent.getAddress(), '/', out_values);

			// Get color channel 
			int channel = std::stoi(out_values.back()) - 1;
			assert(channel <= 4 && channel >= 0);

			// Update color
			updateColor(oscEvent, channel);
			return;
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/rotation"))
		{
			updateRotate(oscEvent);
			return;
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/resetrotation"))
		{
			resetRotate(oscEvent);
			return;
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/selection"))
		{
			// Get index
			std::vector<std::string> out_values;
			utility::gSplitString(oscEvent.getAddress(), '/', out_values);

			assert(out_values.size() == 3);
			int index = math::clamp<int>(std::stoi(out_values.back())-1, 0,1);
			setIndex(oscEvent, index);
			return;
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/blend"))
		{
			std::vector<std::string> out_values;
			utility::gSplitString(oscEvent.getAddress(), '/', out_values);
			assert(out_values.size() == 3);
			int index = math::clamp<int>(std::stoi(out_values.back()) - 1, 0, 1);
			setBlend(oscEvent, index);
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/scale"))
		{
			setScale(oscEvent);
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/position"))
		{
			setPosition(oscEvent);
		}
	}


	void OSCLaserInputHandlerInstance::updateColor(const OSCEvent& oscEvent, int channel)
	{
		// New value
		assert(oscEvent[0].isFloat());
		float v = oscEvent[0].asFloat();

		// Update color channel
		mColorComponent->mColor[channel] = v;
	}


	void OSCLaserInputHandlerInstance::updateRotate(const OSCEvent& oscEvent)
	{
		// New value
		assert(oscEvent.getArgument(0).isFloat());
		float v = oscEvent.getArgument(0).asFloat();
		
		// Get index
		std::vector<std::string> parts;
		utility::gSplitString(oscEvent.getAddress(), '/', parts);
		
		// Get last
		int idx = std::stoi(parts.back().c_str());

		switch (idx)
		{
		case 1:
			mRotateComponent->mProperties.mAxis.x = v;
			break;
		case 2:
			mRotateComponent->mProperties.mAxis.y = v;
			break;
		case 3:
			mRotateComponent->mProperties.mAxis.z = v;
			break;
		case 4:
			mRotateComponent->mProperties.mSpeed = math::power<float>(v, 4.0f);
			break;
		default:
			assert(false);
		}
	}


	void OSCLaserInputHandlerInstance::resetRotate(const OSCEvent& event)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		if (v < 0.99f)
			return;

		mRotateComponent->reset();
		mRotateComponent->mProperties.mSpeed = 0.0f;
	}


	void OSCLaserInputHandlerInstance::setIndex(const OSCEvent& event, int index)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		// Get line to update
		LineSelectionComponentInstance* selector = index == 0 ? mSelectorOne : mSelectorTwo;

		// Map value to range
		float count = static_cast<float>(selector->getCount());
		int idx = math::min<int>(static_cast<int>(count * v), count - 1);
		selector->setIndex(idx);
	}


	void OSCLaserInputHandlerInstance::setBlend(const OSCEvent& event, int index)
	{	
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		if (index == 0)
		{
			mBlendComponent->mBlendSpeed = math::power<float>(v, 1.05f);
			return;
		}
		mBlendComponent->mBlendValue = v;
	}


	void OSCLaserInputHandlerInstance::setScale(const OSCEvent& event)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		mTransformComponent->setUniformScale(math::fit<float>(v, 0.0f, 1.0f, 0.0f, 3.0f) * mInitialScale);
	}


	void OSCLaserInputHandlerInstance::setPosition(const OSCEvent& event)
	{
		assert(event.getCount() == 2);
		float pos_x = event[1].asFloat();
		float pos_y = event[0].asFloat();

		float fru_x = mLaserOutput->mProperties.mFrustrum.x / 2.0f;
		float fru_y = mLaserOutput->mProperties.mFrustrum.y / 2.0f;

		glm::vec3 current_xform = mTransformComponent->getTranslate();
		current_xform.x = math::lerp<float>(fru_x*-1.0f, fru_x, pos_x);
		current_xform.y = math::lerp<float>(fru_y*-1.0f, fru_y, pos_y);

		mTransformComponent->setTranslate(current_xform);
	}


	void OSCLaserInputHandler::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RotateComponent));
		components.emplace_back(RTTI_OF(nap::OSCInputComponent));
		components.emplace_back(RTTI_OF(nap::LineBlendComponent));
		components.emplace_back(RTTI_OF(nap::LineColorComponent));
	}

}
