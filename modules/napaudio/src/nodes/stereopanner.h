#pragma once

#include "audionode.h"

namespace nap {
    
    namespace audio {
        
        /**
         * Node to perform equal power panning on a stereo signal
         */
        class NAPAPI StereoPanner : public AudioNode {
        public:
            StereoPanner(AudioNodeManager& manager);
            
            /**
             * @param value: 0 is far left, 1 is far right
             */
            void setPanning(ControllerValue value);
            
            /**
             * Left channel of the stereo input signal
             */
            AudioInput leftInput;
            
            /**
             * Right channel of the stereo input signal
             */
            AudioInput rightInput;
            
            /**
             * Left channel of the stereo output signal
             */
            AudioOutput leftOutput = { this, &StereoPanner::calculateLeft };
            
            /**
             * Right channel of the stereo output signal
             */
            AudioOutput rightOutput = { this, &StereoPanner::calculateRight };
            
        private:
            void calculateLeft(SampleBuffer& buffer);
            
            void calculateRight(SampleBuffer& buffer);
            
            ControllerValue mPanning = 0.5f;
            ControllerValue mLeftGain = 0;
            ControllerValue mRightGain = 0;
        };
    
    }

}
