#include "midieventqueue.h"

// Nap includes
#include <entity.h>
#include <midiinputcomponent.h>

// RTTI
RTTI_BEGIN_CLASS(nap::MidiEventQueueComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MidiEventQueueComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    void MidiEventQueueComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        components.emplace_back(RTTI_OF(nap::MidiInputComponent));

    }

    
    bool MidiEventQueueComponentInstance::init(utility::ErrorState& errorState)
    {
        auto& midiInputComponent = getEntityInstance()->getComponent<MidiInputComponentInstance>();
        midiInputComponent.messageReceived.connect(eventReceivedSlot);
        return true;
    }
    
    
    std::vector<std::unique_ptr<MidiEvent>> MidiEventQueueComponentInstance::poll()
    {
        std::vector<std::unique_ptr<MidiEvent>> result;
        for (auto& event : mReceivedEvents)
            result.emplace_back(std::move(event));
        mReceivedEvents.clear();
        return result;
    }

    
    void MidiEventQueueComponentInstance::onEventReceived(const MidiEvent& event)
    {
        mReceivedEvents.emplace_back(std::make_unique<MidiEvent>(event.getData(), event.getPort()));
    }

        
}
