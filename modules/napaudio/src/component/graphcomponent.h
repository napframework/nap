#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <component/audiocomponent.h>
#include <graph/audiograph.h>

namespace nap
{
    
    namespace audio
    {
    
        class GraphComponentInstance;
        
        
        class NAPAPI GraphComponent : public AudioComponent
        {
            RTTI_ENABLE(AudioComponent)
            DECLARE_COMPONENT(GraphComponent, GraphComponentInstance)
            
        public:
            GraphComponent() : AudioComponent() { }
            
            ObjectPtr<Graph> mGraph;
        };

        
        class NAPAPI GraphComponentInstance : public AudioComponentInstance
        {
            RTTI_ENABLE(AudioComponentInstance)
        public:
            GraphComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            OutputPin& getOutputForChannel(int channel) override;
            int getChannelCount() const override;
            
            GraphInstance mGraphInstance;
        };
        
    }
    
}
