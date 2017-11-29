#pragma once

// Nap includes
#include <component.h>

#include <audio/node/levelmeternode.h>
#include <audio/core/audioobject.h>

namespace nap
{
    
    namespace audio
    {
        
        class LevelMeterComponentInstance;
        
        
        class NAPAPI LevelMeterComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(LevelMeterComponent, LevelMeterComponentInstance)
            
        public:
            LevelMeterComponent() : Component() { }
            
            nap::ObjectPtr<AudioObject> mInput;
            
        private:
        };
        
        
        class NAPAPI LevelMeterComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            LevelMeterComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            NodeManager& getNodeManager();
            
            SampleValue getLevel(int channel);
            
        private:
            AudioObjectInstance* mInput = nullptr;
            std::vector<std::unique_ptr<LevelMeterNode>> meters;
        };
        
    }
    
}
