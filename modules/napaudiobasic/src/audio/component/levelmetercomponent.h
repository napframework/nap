#pragma once

// Nap includes
#include <component.h>

// Audio includes
#include <audio/node/levelmeternode.h>
#include <audio/component/audiocomponentbase.h>

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
            
            nap::ComponentPtr<AudioComponentBase> mInput;
            TimeValue mAnalysisWindowSize = 10;
            LevelMeterNode::Type mMeterType = LevelMeterNode::Type::RMS;
            
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
            nap::ComponentInstancePtr<AudioComponentBase> mInput = { this, &LevelMeterComponent::mInput };
            std::vector<std::unique_ptr<LevelMeterNode>> meters;
        };
        
    }
    
}
