#include "randomoschandler.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <utility/stringutils.h>

// nap::randomoschandler run time class definition 
RTTI_BEGIN_CLASS(nap::RandomOSCHandler)
	RTTI_PROPERTY("CombinationComponent", &nap::RandomOSCHandler::mCombinationComponent, nap::rtti::EPropertyMetaData::Required)
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
		// Get the osc input component, should be there because this guy depends on it
		mOSCInputComponent = &(getEntityInstance()->getComponent<OSCInputComponentInstance>());
		assert(mOSCInputComponent != nullptr);

		// Connect osc received signal to our slot that forward the input to the handleMessageReceived Function
		mOSCInputComponent->messageReceived.connect(eventReceivedSlot);

		// Populate our map of callbacks
		mOscEventFuncs.emplace(std::make_pair("brightness", &RandomOSCHandlerInstance::updateBrightness));

		return true;
	}


	void RandomOSCHandlerInstance::update(double deltaTime)
	{

	}


	void RandomOSCHandlerInstance::onEventReceived(const nap::OSCEvent& oscEvent)
	{
		std::vector<std::string> addressParts;
		utility::splitString(oscEvent.getAddress(), '/', addressParts);
		assert(addressParts.size() > 1);
		assert(addressParts.front() == "light-grid");

		addressParts.erase(addressParts.begin());

		auto it = mOscEventFuncs.find(addressParts[1]);
		if (it == mOscEventFuncs.end())
		{
			nap::Logger::warn("unknown osc event: %s", oscEvent.getAddress().c_str());
			return;
		}

		// Call found callback
		addressParts.erase(addressParts.begin(), addressParts.begin() + 2);
		(this->*(it->second))(oscEvent, addressParts);
	}

	void RandomOSCHandlerInstance::updateBrightness(const OSCEvent& event, const std::vector<std::string>& args) {

	}
}
