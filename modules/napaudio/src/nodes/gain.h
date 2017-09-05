#pragma once

#include <audionode.h>

namespace nap {
    
    namespace audio {
    
        /**
         * Node to scale an audio signal
         */
        class NAPAPI Gain : public AudioNode {
        public:
            Gain(AudioNodeManager& manager) : AudioNode(manager) { }
            
            /**
             * The input to be scaled
             */
            AudioInput audioInput;
            
            /**
             * Outputs the scaled signal
             */
            AudioOutput audioOutput = { this, &Gain::calculate };
            
            /**
             * Sets the gain or scaling factor
             */
            void setGain(ControllerValue gain) { mGain = gain; }
            
            /**
             * @return: the gain scaling factor
             */
            ControllerValue getGain() const { return mGain; }
            
        private:
            void calculate(SampleBuffer& buffer)
            {
                SampleBuffer& inputBuffer = *audioInput.pull();
                
                for (auto i = 0; i < buffer.size(); ++i)
                    buffer[i] = inputBuffer[i] * mGain;
            }
            
            ControllerValue mGain  = 1;
        };
        
    }
}
