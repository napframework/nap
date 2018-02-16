#pragma once

#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Node to perform equal power panning on a stereo signal
         */
        class NAPAPI StereoPannerNode : public Node
        {
        public:
            StereoPannerNode(NodeManager& manager);
            
            /**
             * @param value: 0 is far left, 1 is far right
             */
            void setPanning(ControllerValue value);
            
            /**
             * Left channel of the stereo input signal
             */
            InputPin leftInput;
            
            /**
             * Right channel of the stereo input signal
             */
            InputPin rightInput;
            
            /**
             * Left channel of the stereo output signal
             */
            OutputPin leftOutput = { this };
            
            /**
             * Right channel of the stereo output signal
             */
            OutputPin rightOutput = { this };
            
        private:
            void process() override;
            
            ControllerValue mPanning = 0.5f;
            ControllerValue mLeftGain = 0;
            ControllerValue mRightGain = 0;
        };
    
    }

}
