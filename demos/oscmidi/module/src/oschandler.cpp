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
        auto& oscInputComponent = getEntityInstance()->getComponent<OSCInputComponentInstance>();
        oscInputComponent.messageReceived.connect(eventReceivedSlot);
        return true;
    }
    
    
    std::vector<std::string> OscHandlerComponentInstance::poll()
    {
        std::vector<std::string> result;
        for (auto& event : mReceivedEvents)
            result.emplace_back(std::move(event));
        mReceivedEvents.clear();
        return result;
    }

    
    void OscHandlerComponentInstance::onEventReceived(const OSCEvent& event)
    {
        std::string message = event.getAddress();
        for (const auto& argument : event.getArguments())
            message.append(" " + argument->toString());
        mReceivedEvents.emplace_back(message);
    }

        
}
