#pragma once

// Audio includes
#include <node/controlnode.h>

namespace nap {
    
    namespace audio {
        
        class NAPAPI EnvelopeGenerator : public ControlNode {
        public:
            struct NAPAPI Segment
            {
                TimeValue duration = 0;
                ControllerValue destination = 0;
                bool durationRelative = false;
                ControlNode::RampMode mode = ControlNode::RampMode::LINEAR;
            };
            using Envelope = std::vector<Segment>;

        public:
            EnvelopeGenerator(NodeManager& manager);
            
            void trigger(Envelope& envelope, TimeValue totalDuration = 0);
            void trigger(Envelope& envelope, int startSegment, int endSegment, ControllerValue startValue = 0, TimeValue totalDuration = 0);
            
            nap::Signal<EnvelopeGenerator&> envelopeFinishedSignal;
            
        private:
            void playSegment(int index);
            
            nap::Slot<ControlNode&> rampFinishedSlot = { this, &EnvelopeGenerator::rampFinished };
            void rampFinished(ControlNode&);
            
            int mCurrentSegment = 0;
            int mEndSegment = 0;
            Envelope* mEnvelope = nullptr;
            TimeValue mTotalRelativeDuration = 0;
        };
        
    }
    
}
