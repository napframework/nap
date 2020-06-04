#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/rampedvalue.h>
#include <Spatial/Audio/MultiChannelWithInput.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Node to scale an audio signal
         */
        class NAPAPI FastGainControlNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            FastGainControlNode(NodeManager& manager) : Node(manager) { }
            
            /**
             * The input to be scaled
             */
            InputPin audioInput = { this };
            
            /**
             * Outputs the scaled signal
             */
            OutputPin audioOutput = { this };
            
            /**
             * Sets the gain or scaling factor
             */
            void setGain(ControllerValue gain, TimeValue rampTime);
            
            /**
             * @return: the gain scaling factor
             */
            ControllerValue getGain() const { return mGain.getValue(); }
            
            Signal<ControllerValue>& getDestinationReachedSignal() { return mGain.destinationReachedSignal; }
            
        private:
            /**
             * Calculate the output, perform the scaling
             */
            void process() override;
            
            RampedValue<ControllerValue> mGain  = { 0 }; // Current multiplication factor of the output signal.
        };
        
        
        using FastGainControl = MultiChannelWithInput<FastGainControlNode>;
        
    }
}

