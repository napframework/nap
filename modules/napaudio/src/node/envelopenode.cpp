#include "envelopenode.h"

namespace nap {
    
    namespace audio {
        
        EnvelopeGenerator::EnvelopeGenerator(NodeManager& manager) : ControlNode(manager)
        {
            rampFinishedSignal.connect(rampFinishedSlot);
        }
        
        
        void EnvelopeGenerator::trigger(Envelope& envelope, TimeValue totalDuration)
        {
            trigger(envelope, 0, envelope.size() - 1, 0, totalDuration);
        }

        
        void EnvelopeGenerator::trigger(Envelope& envelope, int startSegment, int endSegment, ControllerValue startValue, TimeValue totalDuration)
        {
            mEnvelope = &envelope;
            mEndSegment = endSegment;
            setValue(startValue);
            
            auto absoluteDuration = 0.f;
            auto relativeDuration = 0.f;
            for (auto i = startSegment; i < endSegment; ++i)
            {
                auto& segment = envelope[i];
                if (!segment.durationRelative)
                    absoluteDuration += segment.duration;
                else
                    relativeDuration += segment.duration;
            }
            
            mTotalRelativeDuration = (totalDuration - absoluteDuration) / relativeDuration;
            if (mTotalRelativeDuration < 0)
                mTotalRelativeDuration = 0;
            
            playSegment(startSegment);
        }
        
        
        void EnvelopeGenerator::playSegment(int index)
        {
            assert(index < mEnvelope->size());
            mCurrentSegment = index;
            auto& segment = (*mEnvelope)[index];
            
            if (segment.durationRelative)
                ramp(segment.destination, segment.duration * mTotalRelativeDuration, segment.mode);
            else
                ramp(segment.destination, segment.duration, segment.mode);
        }
        
        
        void EnvelopeGenerator::rampFinished(ControlNode&)
        {
            if (mCurrentSegment < mEndSegment)
                playSegment(mCurrentSegment + 1);
            else {
                envelopeFinishedSignal(*this);
            }
        }


    }
    
}
