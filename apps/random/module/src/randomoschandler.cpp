#include "randomoschandler.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <utility/stringutils.h>

// nap::randomoschandler run time class definition 
RTTI_BEGIN_CLASS(nap::RandomOSCHandler)
	RTTI_PROPERTY("CombinationComponent", &nap::RandomOSCHandler::mCombinationComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("UpdateMaterialComponent", &nap::RandomOSCHandler::mUpdateMaterialComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ControlGroups", &nap::RandomOSCHandler::mControlGroups, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::randomoschandlerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RandomOSCHandlerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RandomOSCHandler::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(OSCInputComponent));
	}


	bool RandomOSCHandlerInstance::init(utility::ErrorState& errorState)
	{
		// Copy over control groups
		mControlGroups = getComponent<RandomOSCHandler>()->mControlGroups.get();

		// Get the osc input component, should be there because this guy depends on it
		mOSCInputComponent = &(getEntityInstance()->getComponent<OSCInputComponentInstance>());
		assert(mOSCInputComponent != nullptr);

		// Connect osc received signal to our slot that forward the input to the handleMessageReceived Function
		mOSCInputComponent->messageReceived.connect(eventReceivedSlot);

		// Populate our map of callbacks
		mOscEventFuncs.emplace(std::make_pair("brightness", &RandomOSCHandlerInstance::updateBrightness));
		mOscEventFuncs.emplace(std::make_pair("control-group-brightness", &RandomOSCHandlerInstance::updateControlGroupBrightness));
		mOscEventFuncs.emplace(std::make_pair("static-temperature", &RandomOSCHandlerInstance::updateStaticTemperature));

		return true;
	}


	void RandomOSCHandlerInstance::update(double deltaTime)
	{

	}


	void RandomOSCHandlerInstance::onEventReceived(const nap::OSCEvent& oscEvent)
	{
		std::vector<std::string> addressParts;
		utility::splitString(oscEvent.getAddress(), '/', addressParts);
		assert(addressParts.size() > 2);
		assert(addressParts[1] == "light-grid");

		addressParts.erase(addressParts.begin(), addressParts.begin() + 2);

		auto it = mOscEventFuncs.find(addressParts[0]);
		if (it == mOscEventFuncs.end())
		{
			nap::Logger::warn("unknown osc event: %s", oscEvent.getAddress().c_str());
			return;
		}

		// Call found callback
		addressParts.erase(addressParts.begin());
		(this->*(it->second))(oscEvent, addressParts);
	}

	void RandomOSCHandlerInstance::updateBrightness(const OSCEvent& oscEvent, const std::vector<std::string>& args)
	{
		mApplyCombinationComponent->mBrightness = oscEvent[0].asFloat();
	}

	void RandomOSCHandlerInstance::updateControlGroupBrightness(const OSCEvent& oscEvent, const std::vector<std::string>& args)
	{
		int controlGroupIndex = std::stoi(args[0]) - 1;
		ControlGroups::ControlGroup* controlGroup = mControlGroups->getGroup(controlGroupIndex);
		controlGroup->mBrightness = oscEvent[0].asFloat();
	}

	void RandomOSCHandlerInstance::updateStaticTemperature(const OSCEvent& oscEvent, const std::vector<std::string>& args)
	{
		*mUpdateMaterialComponent->getStaticWarmthPtr() = oscEvent[0].asFloat();
	}
}
