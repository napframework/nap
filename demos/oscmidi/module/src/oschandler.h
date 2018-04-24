#pragma once

// Nap includes
#include <component.h>
#include <nap/signalslot.h>
#include <oscevent.h>

namespace nap
{
    
    class OscHandlerComponentInstance;
    
    
    class NAPAPI OscHandlerComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(OscHandlerComponent, OscHandlerComponentInstance)
        
    public:
        OscHandlerComponent() : Component() { }
        
        /**
         * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
         * @param components the components this object depends on
         */
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        
    private:
    };

    
    class NAPAPI OscHandlerComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        OscHandlerComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        
        std::vector<std::string> poll();
        
    private:
        Slot<const OSCEvent&> eventReceivedSlot = { this, &OscHandlerComponentInstance::onEventReceived };
        void onEventReceived(const OSCEvent&);
        
        std::vector<std::string> mReceivedEvents;
    };
        
}
