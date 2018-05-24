#pragma once


// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/delay.h>
#include <audio/utility/linearramper.h>

namespace nap
{
    
    namespace audio
    {
    
        /**
         * Delay line with feedback and dry/wet control.
         */
        class NAPAPI DelayNode : public Node
        {
        public:
            DelayNode(NodeManager& manager, int delayLineSize = 65536 * 8) : Node(manager), mDelay(delayLineSize) { }
            
            InputPin input; /**< The audio input receiving the signal to be delayed. */
            OutputPin output = { this }; /**< The audio output with the processed signal. */
            
            /**
             * Sets the delay time in milliseconds. Ramp time specifies the time it takes to reach the new value.
             */
            void setTime(TimeValue value, TimeValue rampTime = 0);
            
            /**
             * Sets the dry wet value in milliseconds. Ramp time specifies the time it takes to reach the new value.
             * 0 means fully dry, 1. means fully wet.
             */
            void setDryWet(ControllerValue value, TimeValue rampTime = 0);
            
            /**
             * Specify the feedback amount of the delay line.
             */
            void setFeedback(ControllerValue value) { mFeedback = value; }
            
            /**
             * Return the current delay time.
             */
            int getTime() const { return mTime; }
            
            /**
             * Return the dry/wet level. 0 means dry, 1. means fully wet.
             */
            ControllerValue getDryWet() const { return mDryWet; }
            
            /**
             * Returns the feedback amount
             */
            ControllerValue getFeedback() const { return mFeedback; }
            
        private:
            void process() override;
            
            Delay mDelay;
            float mTime = 0; // in samples
            ControllerValue mDryWet = 0.5f;
            ControllerValue mFeedback = 0.f;
            
            LinearRamper<float> mTimeRamper = { mTime };
            LinearRamper<ControllerValue> mDryWetRamper = { mDryWet };
        };
        
    }
    
}
