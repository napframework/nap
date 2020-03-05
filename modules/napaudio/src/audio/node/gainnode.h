#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
    
    namespace audio
    {
    
        /**
         * Node to scale an audio signal
         */
        class NAPAPI GainNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            GainNode(NodeManager& manager, ControllerValue initValue = 1, unsigned int stepCount = 64) : Node(manager), mGain(initValue, stepCount) { }
            
            /**
             * The input to be scaled
             */
            MultiInputPin inputs = { this };
            
            /**
             * Outputs the scaled signal
             */
            OutputPin audioOutput = { this };
            
            /**
             * Sets the gain or scaling factor
             */
            void setGain(ControllerValue gain, TimeValue smoothTime);
            
            /**
             * @return: the gain scaling factor
             */
            ControllerValue getGain() const { return mGain.getValue(); }
            
        private:
            /**
             * Calculate the output, perform the scaling
             */
            void process() override;
            
            LinearSmoothedValue<ControllerValue> mGain; // Current multiplication factor of the output signal.
        };
        
    }
}
