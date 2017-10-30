#pragma once

#include "midievent.h"
#include "midiservice.h"

// Nap includes
#include <nap/component.h>
#include <nap/signalslot.h>


namespace nap
{
    
    class MidiInputComponentInstance;
    
    
    class NAPAPI MidiInputComponent : public nap::Component
    {
        RTTI_ENABLE(nap::Component)
        DECLARE_COMPONENT(MidiInputComponent, MidiInputComponentInstance)
        
    public:
        MidiInputComponent() : nap::Component() { }
        
        std::vector<MidiValue> mPorts;
        std::vector<MidiValue> mChannels;
        std::vector<MidiValue> mNumbers;
        std::vector<MidiEvent::Type> mTypes;
    };

    
    class NAPAPI MidiInputComponentInstance : public nap::ComponentInstance
    {
        RTTI_ENABLE(nap::ComponentInstance)
        friend class MidiService;
        
    public:
        MidiInputComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
        
        virtual ~MidiInputComponentInstance();
                
        nap::Signal<const MidiEvent&> messageReceived;
        
        std::vector<MidiValue> mPorts;
        std::vector<MidiValue> mChannels;
        std::vector<MidiValue> mNumbers;
        std::vector<MidiEvent::Type> mTypes;
        
    protected:
        /**
         * This is triggered by the service when a new midi message is received
         */
        void trigger(const MidiEvent& event) { messageReceived(event); }
        
    private:
        MidiService* mService = nullptr;
    };
    
}
