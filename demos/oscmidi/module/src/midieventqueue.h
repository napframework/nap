#pragma once

// Nap includes
#include <component.h>
#include <nap/signalslot.h>
#include <midievent.h>

namespace nap
{
    
    class MidiEventQueueComponentInstance;
    
    
    class NAPAPI MidiEventQueueComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(MidiEventQueueComponent, MidiEventQueueComponentInstance)
        
    public:
        MidiEventQueueComponent() : Component() { }
        
        /**
         * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
         * @param components the components this object depends on
         */
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        
    private:
    };

    
    class NAPAPI MidiEventQueueComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        MidiEventQueueComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        
        std::vector<std::unique_ptr<MidiEvent>> poll();
        
    private:
        Slot<const MidiEvent&> eventReceivedSlot = { this, &MidiEventQueueComponentInstance::onEventReceived };
        void onEventReceived(const MidiEvent&);
        
        std::vector<std::unique_ptr<MidiEvent>> mReceivedEvents;
    };
        
}
