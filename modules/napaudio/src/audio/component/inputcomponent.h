#pragma once

// Nap includes
#include <component.h>

// Audio includes
#include <audio/core/audionodeptr.h>
#include <audio/component/audiocomponentbase.h>
#include <audio/node/inputnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class AudioInputComponentInstance;
        
        
        /**
         * Component to receive audio input from the audio interface.
         * Can be used as input to an @OutpuComponent of @LevelMeterComponent.
         */
        class NAPAPI AudioInputComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(InputComponent, AudioInputComponentInstance)
            
        public:
            AudioInputComponent() : AudioComponentBase() { }
            
            // Properties
            std::vector<int> mChannels; ///< property: 'Channels' Defines what audio input channels to receive data from. The size of this array determines the number of channels that this component will output.
            
        private:
        };

        
        /**
         * Instance of component to receive audio input from the audio interface.
         * Can be used as input to an @OutpuComponent of @LevelMeterComponent.
         */
        class NAPAPI AudioInputComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)
        public:
            AudioInputComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }
            
            // Inherited from ComponentInstance
            bool init(utility::ErrorState& errorState) override;
            
            // Inherited from AudioComponentBaseInstance
            int getChannelCount() const override { return mInputNodes.size(); }
            OutputPin& getOutputForChannel(int channel) override { return mInputNodes[channel]->audioOutput; }
            
        private:
            std::vector<NodePtr<InputNode>> mInputNodes; // Nodes pulling audio input data out of the ADC inputs from the node manager
        };
        
    }
    
}
