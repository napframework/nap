#include "oschandler.h"

// Nap includes
#include <entity.h>
#include <oscinputcomponent.h>

// RTTI
RTTI_BEGIN_CLASS(nap::OscHandlerComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OscHandlerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    void OscHandlerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.emplace_back(RTTI_OF(nap::OSCInputComponent));

    }

    
    bool OscHandlerComponentInstance::init(utility::ErrorState& errorState)
    {
		mReceivedEvents.reserve(25);
        auto& oscInputComponent = getEntityInstance()->getComponent<OSCInputComponentInstance>();
        oscInputComponent.messageReceived.connect(eventReceivedSlot);
        return true;
    }
    
    
    const std::vector<std::string>& OscHandlerComponentInstance::getMessages()
    {
		return mReceivedEvents;
    }

    
    void OscHandlerComponentInstance::onEventReceived(const OSCEvent& event)
    {
		// Convert and push back the message
		std::string message(event.getAddress());
		for (const auto& argument : event.getArguments())
		{
			// Add
			mReceivedEvents.emplace_back(message + " " + argument->toString());

			// Remove first element when out of range
			if (mReceivedEvents.size() > 25)
			{
				mReceivedEvents.erase(mReceivedEvents.begin());
			}
		}
    }

        
}
