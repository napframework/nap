#include "osclaserinputhandler.h"

#include <nap/entity.h>
#include <polyline.h>
#include <utility/stringutils.h>
#include <mathutils.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::OSCLaserInputHandler)
	RTTI_PROPERTY("LaserOutputComponent",  &nap::OSCLaserInputHandler::mLaserOutputComponent,  nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PrintColor",			   &nap::OSCLaserInputHandler::mPrintColor,				nap::rtti::EPropertyMetaData::Default)
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

		mXformSmoother = getEntityInstance()->findComponent<nap::XformSmoothComponentInstance>();
		if (!errorState.check(mXformSmoother != nullptr, "missing xform smoother"))
			return false;

		mPrintColor = getComponent<OSCLaserInputHandler>()->mPrintColor;

		mInputComponent->messageReceived.connect(mMessageReceivedSlot);

		// Populate our map of callbacks
		mLaserEventFuncs.emplace(std::make_pair("startposition", &OSCLaserInputHandlerInstance::updateStartColor));
		mLaserEventFuncs.emplace(std::make_pair("endposition", &OSCLaserInputHandlerInstance::updateEndColor));
		mLaserEventFuncs.emplace(std::make_pair("intensity", &OSCLaserInputHandlerInstance::setIntensity));
		mLaserEventFuncs.emplace(std::make_pair("rotation", &OSCLaserInputHandlerInstance::updateRotate));
		mLaserEventFuncs.emplace(std::make_pair("resetrotation", &OSCLaserInputHandlerInstance::resetRotate));
		mLaserEventFuncs.emplace(std::make_pair("blend", &OSCLaserInputHandlerInstance::setBlend));
		mLaserEventFuncs.emplace(std::make_pair("scale", &OSCLaserInputHandlerInstance::setScale));
		mLaserEventFuncs.emplace(std::make_pair("modulation", &OSCLaserInputHandlerInstance::setModulation));
		mLaserEventFuncs.emplace(std::make_pair("noise", &OSCLaserInputHandlerInstance::setNoise));
		mLaserEventFuncs.emplace(std::make_pair("synccolor", &OSCLaserInputHandlerInstance::setColorSync));
		mLaserEventFuncs.emplace(std::make_pair("tracer", &OSCLaserInputHandlerInstance::updateTracer));
		mLaserEventFuncs.emplace(std::make_pair("resettracer", &OSCLaserInputHandlerInstance::resetTracer));
		mLaserEventFuncs.emplace(std::make_pair("nextline", &OSCLaserInputHandlerInstance::selectNextLine));
		mLaserEventFuncs.emplace(std::make_pair("random", &OSCLaserInputHandlerInstance::toggleRandom));
		mLaserEventFuncs.emplace(std::make_pair("resetblend", &OSCLaserInputHandlerInstance::resetBlend));
		mLaserEventFuncs.emplace(std::make_pair("startxposition", &OSCLaserInputHandlerInstance::updateXStartColor));
		mLaserEventFuncs.emplace(std::make_pair("startyposition", &OSCLaserInputHandlerInstance::updateYStartColor));
		mLaserEventFuncs.emplace(std::make_pair("endxposition", &OSCLaserInputHandlerInstance::updateXEndColor));
		mLaserEventFuncs.emplace(std::make_pair("endyposition", &OSCLaserInputHandlerInstance::updateYEndColor));
		mLaserEventFuncs.emplace(std::make_pair("smoothx", &OSCLaserInputHandlerInstance::setColorSmoothX));
		mLaserEventFuncs.emplace(std::make_pair("smoothy", &OSCLaserInputHandlerInstance::setColorSmoothY));
		mLaserEventFuncs.emplace(std::make_pair("positionx", &OSCLaserInputHandlerInstance::setPositionX));
		mLaserEventFuncs.emplace(std::make_pair("positiony", &OSCLaserInputHandlerInstance::setPositionY));
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
		assert(event.getCount() == 2);
		glm::vec2 new_pos;
		new_pos.x = math::clamp<float>(event[0].asFloat(), 0.0f, 1.0f);
		new_pos.y = math::clamp<float>(event[1].asFloat(), 0.0f, 1.0f);
		updateColor(new_pos, 0);
	}


	void OSCLaserInputHandlerInstance::updateEndColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event.getCount() == 2);
		glm::vec2 new_pos;
		new_pos.x = math::clamp<float>(event[0].asFloat(), 0.0f, 1.0f);
		new_pos.y = math::clamp<float>(event[1].asFloat(), 0.0f, 1.0f);
		updateColor(new_pos, 1);
	}


	void OSCLaserInputHandlerInstance::updateXStartColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event.getCount() == 1);
		glm::vec2 new_pos = mColorComponent->getStartPosition();
		new_pos.x = math::clamp<float>(event[0].asFloat(), 0.0f, 1.0f);
		updateColor(new_pos, 0);
	}


	void OSCLaserInputHandlerInstance::updateYStartColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event.getCount() == 1);
		glm::vec2 new_pos = mColorComponent->getStartPosition();
		new_pos.y = math::clamp<float>(event[0].asFloat(), 0.0f, 1.0f);
		updateColor(new_pos, 0);
	}


	void OSCLaserInputHandlerInstance::updateXEndColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event.getCount() == 1);
		glm::vec2 new_pos = mColorComponent->getEndPosition();
		new_pos.x = math::clamp<float>(event[0].asFloat(), 0.0f, 1.0f);
		updateColor(new_pos, 1);
	}


	void OSCLaserInputHandlerInstance::updateYEndColor(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event.getCount() == 1);
		glm::vec2 new_pos = mColorComponent->getEndPosition();
		new_pos.y = math::clamp<float>(event[0].asFloat(), 0.0f, 1.0f);
		updateColor(new_pos, 1);
	}


	void OSCLaserInputHandlerInstance::updateColor(const glm::vec2& loc, int position)
	{
		// Will hold the pixel color
		glm::vec3 mp_clr;

		// Set start / end position based on uv coordinates
		if (position == 0)
		{
			mColorComponent->setStartPosition(loc);
			mColorComponent->getColor(loc, mp_clr);
		}
		else
		{
			mColorComponent->setEndPosition(loc);
			mColorComponent->getColor(loc, mp_clr);
		}
		
		if (mPrintColor)
		{
			// Convert color to 8 bit value
			int r = static_cast<int>(mp_clr.x * 255.0f);
			int g = static_cast<int>(mp_clr.y * 255.0f);
			int b = static_cast<int>(mp_clr.z * 255.0f);
			std::cout << "Pixel Color: " << r << " " << g << " " << b << "\n";
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


	void OSCLaserInputHandlerInstance::setColorSmoothX(const OSCEvent& event, const std::vector<std::string>& args)
	{
		if (!(event[0].isInt()))
		{
			nap::Logger::warn("expected an int value associated with message: %s, got: %s instead", event.getAddress().c_str(), event[0].getValueType().get_name().data());
			return;
		}

		float secs = static_cast<float>(event[0].asInt()) / 1000.0f;
		mColorComponent->setStartSmoothSpeedX(secs);
	}


	void OSCLaserInputHandlerInstance::setColorSmoothY(const OSCEvent& event, const std::vector<std::string>& args)
	{
		if (!(event[0].isInt()))
		{
			nap::Logger::warn("expected an int value associated with message: %s, got: %s instead", event.getAddress().c_str(), event[0].getValueType().get_name().data());
			return;
		}

		float secs = static_cast<float>(event[0].asInt()) / 1000.0f;
		mColorComponent->setStartSmoothSpeedY(secs);
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
		{
			float vs = math::power<float>(abs(v), 4.0f);
			vs *= math::sign<float>(v);
			mRotateComponent->mProperties.mSpeed = vs;
			break;
		}
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
		v = math::max<float>(v*2.0f, 0.05f) * mInitialScale;
		mXformSmoother->setTargetScale(v);
	}


	void OSCLaserInputHandlerInstance::setPosition(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event.getCount() == 2);
		float pos_x = event[0].asFloat();
		float pos_y = event[1].asFloat();

		assert(mLaserOutput != nullptr);
		float fru_x = mLaserOutput->mProperties.mFrustum.x / 2.0f;
		float fru_y = mLaserOutput->mProperties.mFrustum.y / 2.0f;

		glm::vec3 current_xform = mXformSmoother->getTarget();
		current_xform.x = math::lerp<float>(fru_x*-1.0f, fru_x, pos_x);
		current_xform.y = math::lerp<float>(fru_y*-1.0f, fru_y, pos_y);

		mXformSmoother->setTarget(current_xform);
	}


	void OSCLaserInputHandlerInstance::setPositionX(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		assert(mLaserOutput != nullptr);
		float fru_x = mLaserOutput->mProperties.mFrustum.x / 2.0f;

		glm::vec3 current_xform = mXformSmoother->getTarget();
		current_xform.x = math::lerp<float>(fru_x*-1.0f, fru_x, v);
		mXformSmoother->setTarget(current_xform);
	}


	void OSCLaserInputHandlerInstance::setPositionY(const OSCEvent& event, const std::vector<std::string>& args)
	{
		assert(event[0].isFloat());
		float v = event[0].asFloat();

		assert(mLaserOutput != nullptr);
		float fru_y = mLaserOutput->mProperties.mFrustum.y / 2.0f;

		glm::vec3 current_xform = mXformSmoother->getTarget();
		current_xform.y = math::lerp<float>(fru_y*-1.0f, fru_y, v);
		mXformSmoother->setTarget(current_xform);
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
		components.emplace_back(RTTI_OF(nap::XformSmoothComponent));
	}

}
