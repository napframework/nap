#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/delay.h>

#include <Spatial/Audio/FastLinearSmoothedValue.h>
#include <Spatial/Audio/MultiChannelWithInput.h>

namespace nap
{
    
    namespace audio
    {
    
        /**
         * Delay line with feedback.
         * Basically a copy of the DelayNode, but here to be able to tweak.
         */
        class NAPAPI FastDelayNode : public Node
        {
            RTTI_ENABLE(Node)
        public:
            FastDelayNode(NodeManager& manager, int delayLineSize = 65536 * 2);
            
            InputPin audioInput = { this }; /**< The audio input receiving the signal to be delayed. */
            OutputPin output = { this }; /**< The audio output with the processed signal. */
            
            
            /**
             * Sets the delay line size. Should not be called after init.
             */
            void setDelayLineSize(int delayLineSize);
            
            /**
             * Sets the delay time in milliseconds. Ramp time specifies the time it takes to reach the new value.
             */
            void setTime(TimeValue value, TimeValue rampTime = 0);
            
            /**
             * Specify the feedback amount of the delay line.
             */
            void setFeedback(ControllerValue value) { mFeedback = value; }
            
            
            /**
             * Sets the polarity of the delay line. If this is set to 'false', the output of this node will be negated.
             */
            void setPolarity(bool positive) { mPositive = positive; };
            
            /**
             * Return the current delay time.
             */
            int getTime() const { return mTime.getValue(); }
            
            /**
             * Returns the feedback amount
             */
            ControllerValue getFeedback() const { return mFeedback; }
            
            /**
             * Clear the delayline by filling the buffer with zero's.
             */
            void clear();
            
        private:
            void process() override;
            
            Delay mDelay;
            FastLinearSmoothedValue<float> mTime = { 0, 44 }; // in samples
            ControllerValue mFeedback = 0.f;
            bool mPositive = true;
        
        };
        
        
        
        /**
         * Object containing a FastDelayNode on each channel.
         */
        class NAPAPI FastDelay : public MultiChannelWithInput<FastDelayNode>
        {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            FastDelay() = default;
            int mDelayLineSize = 65536 * 2;
            
        private:
            virtual bool onInitNode(int channel, FastDelayNode& node, utility::ErrorState& errorState) override
            {
                node.setDelayLineSize(mDelayLineSize);
                return true;
            }
            
        };
        
    }
    
}
