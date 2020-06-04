#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/linearsmoothedvalue.h>

#include <Spatial/Audio/FastLinearSmoothedValue.h>

namespace nap
{
    
    namespace audio
    {
    
        /**
         * Node to scale an audio signal
         */
        class NAPAPI FastGainNode : public Node
        {
        public:
            FastGainNode(NodeManager& manager) : Node(manager) { }

            FastGainNode(NodeManager& manager, ControllerValue gain, TimeValue rampTime);
            
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
            void setGain(ControllerValue gain) { mGain.setValue(gain); }
            
            /**
             * @return: the gain scaling factor
             */
            ControllerValue getGain() const { return mGain.getValue(); }

            /**
             * Set the duration of the smoothing ramp when the value changes.
             */
            void setRampTime(TimeValue rampTime);
            
        private:
            /**
             * Calculate the output, perform the scaling
             */
            void process() override;
            
            FastLinearSmoothedValue<ControllerValue> mGain = { 0, 64 };
			
            bool mOutputIsZero = true;
        };
        
    }
}
