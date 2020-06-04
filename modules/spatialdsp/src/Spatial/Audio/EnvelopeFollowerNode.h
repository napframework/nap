#pragma once

// Audio includes
#include <audio/core/audionode.h>


namespace nap
{
    
    namespace audio
    {
        
        /**
         * Envelope follower with attack/release controls. The signal passes through this node, unaltered.
         */
        class NAPAPI EnvelopeFollowerNode : public Node
        {
        public:
            EnvelopeFollowerNode(NodeManager& manager) : Node(manager) {
                setAttack(1.);
                setRelease(1.);
            }
            
            /**
             * The audio input.
             */
            InputPin audioInput = { this };
            
            /**
             * The audio output.
             */
            OutputPin audioOutput = { this };
            
            /**
             * Sets the attack time (in ms).
             */
            void setAttack(ControllerValue attackInMs){
                mAttackCoef = exp(log(0.01)/( attackInMs * getSampleRate() * 0.001));
            }
            
            /**
             * Sets the release time (in ms).
             */
            void setRelease(ControllerValue releaseInMs){
                mReleaseCoef = exp(log(0.01)/( releaseInMs * getSampleRate() * 0.001));
            }
            
            /**
             * @return: the current measured value.
             */
            ControllerValue getValue() const { return mValue; }
            
            
        private:
            void process() override;
            
            std::atomic<ControllerValue> mValue = { 0.0 };
            std::atomic<ControllerValue> mAttackCoef = { 0. };
            std::atomic<ControllerValue> mReleaseCoef = { 0. };
        };
        
    }
}
