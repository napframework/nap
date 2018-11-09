#include "randomoschandler.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

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

		return true;
	}


	void RandomOSCHandlerInstance::update(double deltaTime)
	{

	}


	void RandomOSCHandlerInstance::onEventReceived(const nap::OSCEvent& oscEvent)
	{
		float v =  oscEvent[0].asFloat();
		mApplyCombinationComponent->mBrightness = v;
		//nap::Logger::info("wooop: %f", v);
	}

}