#include "envelopenode.h"

// RTTI include
#include <rtti/rtti.h>

// RTTI
RTTI_BEGIN_ENUM(nap::audio::ControlNode::RampMode)
    RTTI_ENUM_VALUE(nap::audio::ControlNode::RampMode::LINEAR, "Linear"),
    RTTI_ENUM_VALUE(nap::audio::ControlNode::RampMode::EXPONENTIAL, "Exponential")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::audio::EnvelopeGenerator::Segment)
    RTTI_PROPERTY("Duration", &nap::audio::EnvelopeGenerator::Segment::mDuration, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Destination", &nap::audio::EnvelopeGenerator::Segment::mDestination, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("DurationRelative", &nap::audio::EnvelopeGenerator::Segment::mDurationRelative, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("RampMode", &nap::audio::EnvelopeGenerator::Segment::mMode, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        EnvelopeGenerator::EnvelopeGenerator(NodeManager& manager) : ControlNode(manager)
        {
            rampFinishedSignal.connect(rampFinishedSlot);
        }
        
        
        void EnvelopeGenerator::trigger(std::shared_ptr<Envelope>& envelope, TimeValue totalDuration)
        {
            trigger(envelope, 0, envelope->size() - 1, 0, totalDuration);
        }

        
        void EnvelopeGenerator::trigger(std::shared_ptr<Envelope>& envelope, int startSegment, int endSegment, ControllerValue startValue, TimeValue totalDuration)
        {
            mEnvelope = envelope;
            mEndSegment = endSegment;
            setValue(startValue);
            
            auto absoluteDuration = 0.f;
            auto relativeDuration = 0.f;
            for (auto i = startSegment; i < endSegment; ++i)
            {
                auto& segment = (*envelope)[i];
                if (!segment.mDurationRelative)
                    absoluteDuration += segment.mDuration;
                else
                    relativeDuration += segment.mDuration;
            }
            
            mTotalRelativeDuration = (totalDuration - absoluteDuration) / relativeDuration;
            if (mTotalRelativeDuration < 0)
                mTotalRelativeDuration = 0;
            
            playSegment(startSegment);
        }
        
        
        void EnvelopeGenerator::stop(TimeValue rampTime)
        {
            mCurrentSegment = mEndSegment;
            ramp(0, rampTime);
        }
        
        
        void EnvelopeGenerator::playSegment(int index)
        {
            assert(index < mEnvelope->size());
            mCurrentSegment = index;
            auto& segment = (*mEnvelope)[index];
            
            if (segment.mDurationRelative)
                ramp(segment.mDestination, segment.mDuration * mTotalRelativeDuration, segment.mMode);
            else
                ramp(segment.mDestination, segment.mDuration, segment.mMode);
        }
        
        
        void EnvelopeGenerator::rampFinished(ControlNode&)
        {
            if (mCurrentSegment < mEndSegment)
                playSegment(mCurrentSegment + 1);
            else {
                if (getValue() == 0)
                    envelopeFinishedSignal(*this);
            }
        }


    }
    
}
