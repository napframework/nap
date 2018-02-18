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
        
        
        /**
         * Component to measure the amplitude level of the audio signal from an @AudioComponentBase
         */
        class NAPAPI LevelMeterComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(LevelMeterComponent, LevelMeterComponentInstance)
            
        public:
            LevelMeterComponent() : Component() { }
            
            nap::ComponentPtr<AudioComponentBase> mInput; /**< The component whose audio output will be measured. */
            TimeValue mAnalysisWindowSize = 10; /**< Size of an analysis window in milliseconds */
            LevelMeterNode::Type mMeterType = LevelMeterNode::Type::RMS; /**< Type of analysis to be used: RMS for root mean square, PEAK for the peak of the analysis window */
            
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
            
            /**
             * Returns the current level for a certain channel
             */
            ControllerValue getLevel(int channel);
            
        private:
            nap::ComponentInstancePtr<AudioComponentBase> mInput = { this, &LevelMeterComponent::mInput };
            std::vector<std::unique_ptr<LevelMeterNode>> meters;
        };
        
    }
    
}
