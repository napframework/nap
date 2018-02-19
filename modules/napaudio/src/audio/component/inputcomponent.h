#pragma once

// Nap includes
#include <component.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/node/inputnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class InputComponentInstance;
        
        
        class NAPAPI InputComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(InputComponent, InputComponentInstance)
            
        public:
            InputComponent() : AudioComponentBase() { }
            
            std::vector<int> mChannels;
            
        private:
        };

        
        class NAPAPI InputComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)
        public:
            InputComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Inherited from ComponentInstance
            bool init(utility::ErrorState& errorState) override;
            
            // Inherited from AudioComponentBaseInstance
            int getChannelCount() const override { return mInputNodes.size(); }
            OutputPin& getOutputForChannel(int channel) override { return mInputNodes[channel]->audioOutput; }
            
        private:
            std::vector<std::unique_ptr<InputNode>> mInputNodes;
        };
        
    }
    
}
