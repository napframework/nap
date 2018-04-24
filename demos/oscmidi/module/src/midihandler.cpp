#include "midihandler.h"

// Nap includes
#include <entity.h>
#include <midiinputcomponent.h>

// RTTI
RTTI_BEGIN_CLASS(nap::MidiHandlerComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MidiHandlerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    void MidiHandlerComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.emplace_back(RTTI_OF(nap::MidiInputComponent));

    }

    
    bool MidiHandlerComponentInstance::init(utility::ErrorState& errorState)
    {
        auto& midiInputComponent = getEntityInstance()->getComponent<MidiInputComponentInstance>();
        midiInputComponent.messageReceived.connect(eventReceivedSlot);
        return true;
    }
    
    
    std::vector<std::string> MidiHandlerComponentInstance::poll()
    {
        std::vector<std::string> result;
        for (auto& event : mReceivedEvents)
            result.emplace_back(event);
        mReceivedEvents.clear();
        return result;
    }

    
    void MidiHandlerComponentInstance::onEventReceived(const MidiEvent& event)
    {
        mReceivedEvents.emplace_back(event.toString());
    }

        
}
