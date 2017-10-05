#include "osclaserinputhandler.h"

#include <nap/entity.h>
#include <polyline.h>
#include <utility/stringutils.h>
#include <mathutils.h>
#include <nap/logger.h>

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

		mTraceComponent = getEntityInstance()->findComponent<nap::LineTraceComponentInstance>();
		if (!errorState.check(mTraceComponent != nullptr, "missing trace component"))
			return false;

		mSwitcher = getEntityInstance()->findComponent<nap::LineAutoSwitchComponentInstance>();
		if (!errorState.check(mSwitcher != nullptr, "missing trace component"))
			return false;

		mLaserOutput = getComponent<OSCLaserInputHandler>()->mLaserOutputComponent.get();

		mInputComponent->messageReceived.connect(mMessageReceivedSlot);

		// Populate our map of callbacks
		mLaserEventFuncs.emplace(std::make_pair("startposition", &OSCLaserInputHandlerInstance::updateStartColor));
		mLaserEventFuncs.emplace(std::make_pair("endposition", &OSCLaserInputHandlerInstance::updateEndColor));
		mLaserEventFuncs.emplace(std::make_pair("intensity", &OSCLaserInputHandlerInstance::setIntensity));
		mLaserEventFuncs.emplace(std::make_pair("rotation", &OSCLaserInputHandlerInstance::updateRotate));
		mLaserEventFuncs.emplace(std::make_pair("resetrotation", &OSCLaserInputHandlerInstance::resetRotate));
		mLaserEventFuncs.emplace(std::make_pair("blend", &OSCLaserInputHandlerInstance::setBlend));
		mLaserEventFuncs.emplace(std::make_pair("scale", &OSCLaserInputHandlerInstance::setScale));
		mLaserEventFuncs.emplace(std::make_pair("position", &OSCLaserInputHandlerInstance::setPosition));
		mLaserEventFuncs.emplace(std::make_pair("modulation", &OSCLaserInputHandlerInstance::setModulation));
		mLaserEventFuncs.emplace(std::make_pair("noise", &OSCLaserInputHandlerInstance::setNoise));
		mLaserEventFuncs.emplace(std::make_pair("synccolor", &OSCLaserInputHandlerInstance::setColorSync));
		mLaserEventFuncs.emplace(std::make_pair("tracer", &OSCLaserInputHandlerInstance::updateTracer));
		mLaserEventFuncs.emplace(std::make_pair("resettracer", &OSCLaserInputHandlerInstance::resetTracer));
		mLaserEventFuncs.emplace(std::make_pair("nextline", &OSCLaserInputHandlerInstance::selectNextLine));
		mLaserEventFuncs.emplace(std::make_pair("random", &OSCLaserInputHandlerInstance::toggleRandom));
		mLaserEventFuncs.emplace(std::make_pair("resetblend", &OSCLaserInputHandlerInstance::resetBlend));
		return true;
	}


	void OSCLaserInputHandlerInstance::handleMessageReceived(const nap::OSCEvent& oscEvent)
	{
		std::vector<std::string> parts;
		utility::gSplitString(oscEvent.getAddress(), '/', parts);
		assert(parts.size() > 2);

		// Erase the first 2 entries
		parts.erase(parts.begin(), parts.begin()+2);

		auto it = mLaserEventFuncs.find(parts[0]);
		if (it == mLaserEventFuncs.end())
		{
			nap::Logger::warn("unknown osc event: %s", oscEvent.getAddress().c_str());
			return;
		}

		// Call found callback
		parts.erase(parts.begin(), parts.begin()+1);
		(this->*(it->second))(oscEvent, parts);
	}


	void OSCLaserInputHandlerInstance::updateStartColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		updateColor(event, 0);
	}


	void OSCLaserInputHandlerInstance::updateEndColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		updateColor(event, 1);
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


	void OSCLaserInputHandlerInstance::resetBlend(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		if (v > 0.99f)
		{
			mBlendComponent->reset();
			mBlendComponent->mBlendSpeed = 0.0f;
			mBlendComponent->mBlendValue = 0.0f;
		}
	}


	void OSCLaserInputHandlerInstance::updateRotate(const OSCEvent& oscEvent, const std::vector<std::string>& args)
	{
		// New value
		assert(oscEvent.getArgument(0).isFloat());
		float v = math::max(oscEvent.getArgument(0).asFloat(), math::epsilon<float>());
		
		// Get last
		int idx = std::stoi(args[0]);

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


	void OSCLaserInputHandlerInstance::resetRotate(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		if (v < 0.99f)
			return;

		mRotateComponent->reset();
		mRotateComponent->mProperties.mSpeed = 0.0f;
		mRotateComponent->mProperties.mOffset = 0.0f;
	}


	void OSCLaserInputHandlerInstance::setBlend(const OSCEvent& event, const std::vector<std::string>& args)
	{	
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		mBlendComponent->mBlendSpeed = math::power<float>(v, 1.05f);
	}


	void OSCLaserInputHandlerInstance::setScale(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		mTransformComponent->setUniformScale(math::fit<float>(v, 0.0f, 1.0f, 0.1f, 3.0f) * mInitialScale);
	}


	void OSCLaserInputHandlerInstance::setPosition(const OSCEvent& event, const std::vector<std::string>& args)
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


	void OSCLaserInputHandlerInstance::setModulation(const OSCEvent& event, const std::vector<std::string>& args)
	{
		// Get index
		int index = std::stoi(args[0]) - 1;
		assert(index < 5 && index >= 0);

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


	void OSCLaserInputHandlerInstance::setNoise(const OSCEvent& event, const std::vector<std::string>& args)
	{
		// Get index
		int index = std::stoi(args[0]) - 1;
		assert(index < 4 && index >= 0);

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


	void OSCLaserInputHandlerInstance::setColorSync(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		bool sync = event[0].asFloat() > math::epsilon<float>();
		mColorComponent->link(sync);
	}


	void OSCLaserInputHandlerInstance::updateTracer(const OSCEvent& event, const std::vector<std::string>& args)
	{
		// Get index
		int index = std::stoi(args[0]) - 1;
		assert(index < 3 && index >= 0);

		// Get value
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		switch (index)
		{
		case 0:
			mTraceComponent->mProperties.mLength = math::fit<float>(v, 0.0f, 1.0f,0.05f, 1.0f);
			break;
		case 1:
			mTraceComponent->mProperties.mSpeed = v;
			break;
		case 2:
			mTraceComponent->mProperties.mOffset = v;
			break;
		default:
			assert(false);
			break;
		}
	}


	void OSCLaserInputHandlerInstance::resetTracer(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		if (v > math::epsilon<float>())
		{
			mTraceComponent->reset();
			mTraceComponent->mProperties.mOffset = 0.0f;
			mTraceComponent->mProperties.mSpeed = 0.0f;
		}
	}


	void OSCLaserInputHandlerInstance::setIntensity(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		mColorComponent->setIntensity(event[0].asFloat());
	}


	void OSCLaserInputHandlerInstance::selectNextLine(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		mSwitcher->setLineIndex(static_cast<int>(v));
	}


	void OSCLaserInputHandlerInstance::toggleRandom(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();
		mSwitcher->setRandom(v > 0.01f);
	}


	void OSCLaserInputHandler::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RotateComponent));
		components.emplace_back(RTTI_OF(nap::OSCInputComponent));
		components.emplace_back(RTTI_OF(nap::LineBlendComponent));
		components.emplace_back(RTTI_OF(nap::LineColorComponent));
		components.emplace_back(RTTI_OF(nap::LineModulationComponent));
		components.emplace_back(RTTI_OF(nap::LineAutoSwitchComponent));
	}

}
