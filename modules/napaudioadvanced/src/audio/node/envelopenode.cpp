#include "envelopenode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

// RTTI include
#include <rtti/rtti.h>

// RTTI
RTTI_BEGIN_ENUM(nap::audio::RampMode)
    RTTI_ENUM_VALUE(nap::audio::RampMode::Linear, "Linear"),
    RTTI_ENUM_VALUE(nap::audio::RampMode::Exponential, "Exponential")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::audio::EnvelopeNode::Segment)
    RTTI_PROPERTY("Duration", &nap::audio::EnvelopeNode::Segment::mDuration, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Destination", &nap::audio::EnvelopeNode::Segment::mDestination, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("DurationRelative", &nap::audio::EnvelopeNode::Segment::mDurationRelative, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("RampMode", &nap::audio::EnvelopeNode::Segment::mMode, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Translate", &nap::audio::EnvelopeNode::Segment::mTranslate, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EnvelopeNode)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        EnvelopeNode::EnvelopeNode(NodeManager& manager, const Envelope& envelope, SafePtr<Translator<ControllerValue>> translator) : Node(manager), mEnvelope(envelope), mTranslator(translator)
        {
            mValue.destinationReachedSignal.connect(rampFinishedSlot);
        }


        void EnvelopeNode::trigger(TimeValue totalDuration)
        {
            trigger(0, mEnvelope.size() - 1, 0, totalDuration);
        }


        void EnvelopeNode::trigger(int startSegment, int endSegment, ControllerValue startValue, TimeValue totalDuration)
        {
            auto absoluteDuration = 0.f;
            auto relativeDuration = 0.f;
            for (auto i = startSegment; i <= endSegment; ++i)
            {
                auto& segment = mEnvelope[i];
                if (!segment.mDurationRelative)
                    absoluteDuration += segment.mDuration;
                else
                    relativeDuration += segment.mDuration;
            }

            mTotalRelativeDuration = (totalDuration - absoluteDuration) / relativeDuration;
            if (mTotalRelativeDuration < 0)
                mTotalRelativeDuration = 0;

            mNewEndSegment.store(endSegment);
            mNewCurrentSegment.store(startSegment);

			mValue.setValue(startValue);
            mIsDirty.set();
        }


        void EnvelopeNode::stop(TimeValue rampTime)
        {
            assert(rampTime > 0.f);
            mNewCurrentSegment.store(mNewEndSegment.load());
            mFadeOutTime.store(rampTime);
            mIsDirty.set();
        }


        void EnvelopeNode::playSegment(int index)
        {
            auto envelope = mEnvelope;

            assert(index < envelope.size());
            mCurrentSegment = index;
            auto& segment = envelope[index];
            mTranslate = segment.mTranslate;

            if (segment.mDurationRelative)
                mValue.ramp(segment.mDestination, segment.mDuration * mTotalRelativeDuration * getNodeManager().getSamplesPerMillisecond(), segment.mMode);
            else
                mValue.ramp(segment.mDestination, segment.mDuration * getNodeManager().getSamplesPerMillisecond(), segment.mMode);
        }


        void EnvelopeNode::updateEnvelope()
        {
            if (mIsDirty.check())
            {
                mCurrentSegment = mNewCurrentSegment.load();
                mEndSegment = mNewEndSegment.load();
                
                auto fadeOutTime = mFadeOutTime.load();
                if (fadeOutTime != 0.f)
                {
                    mFadeOutTime.store(0.f);
                    mValue.ramp(0.f, fadeOutTime * getNodeManager().getSamplesPerMillisecond(), RampMode::Linear);
                }
                else {
                    if (mCurrentSegment <= mEndSegment)
                        playSegment(mCurrentSegment);
                }
            }
        }


        void EnvelopeNode::process()
        {
            updateEnvelope();
            auto& outputBuffer = getOutputBuffer(output);

            if (mTranslate && mTranslator != nullptr)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    outputBuffer[i] = mTranslator->translate(mValue.getNextValue());
                }
            }
            else {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                {
                    outputBuffer[i] = mValue.getNextValue();
                }
            }
            mCurrentValue.store(outputBuffer.back());
        }


        void EnvelopeNode::rampFinished(ControllerValue value)
        {
            segmentFinishedSignal(*this);
            if (mCurrentSegment < mEndSegment)
                playSegment(mCurrentSegment + 1);
            else {
                if (value == 0.f)
                {
                    mCurrentValue.store(0.f);
                    envelopeFinishedSignal(*this);
                }
            }
        }

    }

}
