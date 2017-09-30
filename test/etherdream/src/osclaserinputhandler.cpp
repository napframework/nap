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
		if(mInputComponent != nullptr)
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

		mModulationComponent = getEntityInstance()->findComponent<nap::LineModulationComponentInstance>();
		if(!errorState.check(mModulationComponent != nullptr, "missing modulation component"))
			return false;

		mNoiseComponent = getEntityInstance()->findComponent<nap::LineNoiseComponentInstance>();
		if (!errorState.check(mNoiseComponent != nullptr, "missing noise component"))
			return false;

		mSelectorOne = getComponent<OSCLaserInputHandler>()->mSelectionComponentOne.get();
		mSelectorTwo = getComponent<OSCLaserInputHandler>()->mSelectionComponentTwo.get();
		mLaserOutput = getComponent<OSCLaserInputHandler>()->mLaserOutputComponent.get();

		mInputComponent->messageReceived.connect(mMessageReceivedSlot);

		return true;
	}


	void OSCLaserInputHandlerInstance::handleMessageReceived(const nap::OSCEvent& oscEvent)
	{
		if (utility::gStartsWith(oscEvent.getAddress(), "/startposition"))
		{
			updateColor(oscEvent, 0);
		}

		if (utility::gStartsWith(oscEvent.getAddress(), "/endposition"))
		{
			updateColor(oscEvent, 1);
		}

		if (utility::gStartsWith(oscEvent.getAddress(), "/intensity"))
		{
			assert(oscEvent[0].isFloat());
			mColorComponent->setIntensity(oscEvent[0].asFloat());
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

		else if (utility::gStartsWith(oscEvent.getAddress(), "/modulation"))
		{
			// Get values
			std::vector<std::string> out_values;
			utility::gSplitString(oscEvent.getAddress(), '/', out_values);

			// Get index
			int index = std::stoi(out_values.back()) - 1;
			assert(index < 5 && index >= 0);
			setModulation(oscEvent, index);
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/noise"))
		{
			// Get values
			std::vector<std::string> out_values;
			utility::gSplitString(oscEvent.getAddress(), '/', out_values);

			// Get index
			int index = std::stoi(out_values.back()) - 1;
			assert(index < 4 && index >= 0);
			setNoise(oscEvent, index);
		}

		else if (utility::gStartsWith(oscEvent.getAddress(), "/synccolor"))
		{
			setColorSync(oscEvent);
		}
	}


	void OSCLaserInputHandlerInstance::updateColor(const OSCEvent& oscEvent, int position)
	{
		assert(oscEvent.getCount() == 2);
		float pos_x = oscEvent[1].asFloat();
		float pos_y = oscEvent[0].asFloat();

		if (position == 0)
		{
			mColorComponent->setStartPosition(glm::vec2(pos_x, pos_y));
			return;
		}
		else
		{
			mColorComponent->setEndPosition(glm::vec2(pos_x, pos_y));
		}
	}


	void OSCLaserInputHandlerInstance::updateRotate(const OSCEvent& oscEvent)
	{
		// New value
		assert(oscEvent.getArgument(0).isFloat());
		float v = math::max(oscEvent.getArgument(0).asFloat(), math::epsilon<float>());
		
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
		case 5:
			mRotateComponent->mProperties.mOffset = v;
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
		mRotateComponent->mProperties.mOffset = 0.0f;
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


	void OSCLaserInputHandlerInstance::setModulation(const OSCEvent& event, int index)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		switch (index)
		{
		case 0:
			mModulationComponent->mProperties.mFrequency = math::fit<float>(math::power<float>(v,3.0f), 0.0f, 1.0f, 0.0f, 10.0f);
			break;
		case 1:
			mModulationComponent->mProperties.mAmplitude = math::power<float>(v, 3.0f);
			break;
		case 2:
			mModulationComponent->mProperties.mSpeed = v;
			break;
		case 3:
			mModulationComponent->mProperties.mOffset = v;
			break;
		case 4:
		{
			int idx = math::min<int>(static_cast<int>(4.0f * v), 3);
			mModulationComponent->mProperties.mWaveform = static_cast<nap::math::EWaveform>(idx);
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void OSCLaserInputHandlerInstance::setNoise(const OSCEvent& event, int index)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		switch (index)
		{
		case 0:
			mNoiseComponent->mProperties.mFrequency = math::fit<float>(math::power<float>(v, 3.0f), 0.0f, 1.0f, 0.0f, 10.0f);
			break;
		case 1:
			mNoiseComponent->mProperties.mAmplitude = math::power<float>(v, 3.0f);
			break;
		case 2:
			mNoiseComponent->mProperties.mSpeed = v;
			break;
		case 3:
			mNoiseComponent->mProperties.mOffset = v;
			break;
		default:
			assert(false);
			break;
		}
	}


	void OSCLaserInputHandlerInstance::setColorSync(const OSCEvent& event)
	{
		assert(event[0].isFloat());
		bool sync = event[0].asFloat() > math::epsilon<float>();
		mColorComponent->link(sync);
	}


	void OSCLaserInputHandler::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RotateComponent));
		components.emplace_back(RTTI_OF(nap::OSCInputComponent));
		components.emplace_back(RTTI_OF(nap::LineBlendComponent));
		components.emplace_back(RTTI_OF(nap::LineColorComponent));
		components.emplace_back(RTTI_OF(nap::LineModulationComponent));
	}

}
