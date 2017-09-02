#pragma once

#include "audionode.h"

namespace nap {
    
    namespace audio {
        
        class StereoPanner : public AudioNode {
        public:
            StereoPanner(AudioNodeManager& manager) : AudioNode(manager)
            {
                setPanning(mPanning);
            }
            
            void setPanning(ControllerValue value)
            {
                mPanning = value;
                mLeftGain = cos(mPanning * 0.5 * pi);
                mRightGain = sin(mPanning * 0.5 * pi);
            }
            
            AudioInput leftInput;
            AudioInput rightInput;
            AudioOutput leftOutput = { this, &StereoPanner::calculateLeft };
            AudioOutput rightOutput = { this, &StereoPanner::calculateRight };
            
        private:
            void calculateLeft(SampleBuffer& buffer)
            {
                SampleBuffer& inputBuffer = *leftInput.pull();
                
                for (auto i = 0; i < buffer.size(); ++i)
                    buffer[i] = inputBuffer[i] * mLeftGain;
            }
            
            void calculateRight(SampleBuffer& buffer)
            {
                SampleBuffer& inputBuffer = *rightInput.pull();
                
                for (auto i = 0; i < buffer.size(); ++i)
                    buffer[i] = inputBuffer[i] * mRightGain;
            }
            
            ControllerValue mPanning = 0.5f;
            ControllerValue mLeftGain = 0;
            ControllerValue mRightGain = 0;
        };
    
    }

}
