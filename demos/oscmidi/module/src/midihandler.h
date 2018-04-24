#pragma once

// Nap includes
#include <component.h>
#include <nap/signalslot.h>
#include <midievent.h>

namespace nap
{
    
    class MidiHandlerComponentInstance;
    
    
    class NAPAPI MidiHandlerComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(MidiHandlerComponent, MidiHandlerComponentInstance)
        
    public:
        MidiHandlerComponent() : Component() { }
        
        /**
         * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
         * @param components the components this object depends on
         */
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        
    private:
    };

    
    class NAPAPI MidiHandlerComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        MidiHandlerComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        
        /**
         * Polls the component for logged events
         */
        std::vector<std::string> poll();
        
    private:
        Slot<const MidiEvent&> eventReceivedSlot = { this, &MidiHandlerComponentInstance::onEventReceived };
        void onEventReceived(const MidiEvent&);
        
        std::vector<std::string> mReceivedEvents;
    };
        
}
