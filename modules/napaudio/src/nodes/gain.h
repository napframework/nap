#pragma once

#include <audionode.h>
#include <utilities/linearramper.h>

namespace nap {
    
    namespace audio {
    
        class Gain : public AudioNode {            
        public:
            Gain(AudioService& service) : AudioNode(service) { }
            
            AudioInput audioInput;
            AudioOutput audioOutput = { this, &Gain::calculate };
            
            void setGain(ControllerValue gain) { mGain = gain; }
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
