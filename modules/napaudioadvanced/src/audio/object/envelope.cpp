#include "envelope.h"

RTTI_BEGIN_CLASS(nap::audio::Envelope)
    RTTI_PROPERTY("Envelope", &nap::audio::Envelope::mSegments, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AutoTrigger", &nap::audio::Envelope::mAutoTrigger, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EqualPowerTable", &nap::audio::Envelope::mEqualPowerTable, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EnvelopeInstance)
    RTTI_FUNCTION("trigger", &nap::audio::EnvelopeInstance::trigger)
    RTTI_FUNCTION("triggerSection", &nap::audio::EnvelopeInstance::triggerSection)
    RTTI_FUNCTION("stop", &nap::audio::EnvelopeInstance::stop)
    RTTI_FUNCTION("setSegmentData", &nap::audio::EnvelopeInstance::setSegmentData)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> Envelope::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            if (mEqualPowerTable == nullptr)
            {
                errorState.fail("No equal power table provided");
                return nullptr;
            }

            auto instance = std::make_unique<EnvelopeInstance>();
            if (!instance->init(mSegments, mAutoTrigger, nodeManager, mEqualPowerTable->getTable(), errorState))
                return nullptr;
            
            return std::move(instance);
        }


        bool EnvelopeInstance::init(EnvelopeNode::Envelope segments, bool autoTrigger, NodeManager& nodeManager, audio::SafePtr<Translator<float>> translator, utility::ErrorState& errorState)
        {
            if (translator == nullptr)
            {
                errorState.fail("No translator provided.");
                return false;
            }
            mTranslator = translator;
            mEnvelopeGenerator = nodeManager.makeSafe<EnvelopeNode>(nodeManager, segments, mTranslator);

            if (autoTrigger)
                mEnvelopeGenerator->trigger();

            return true;
        }


        void EnvelopeInstance::setSegmentData(unsigned int segmentIndex, TimeValue duration, ControllerValue destination, bool durationRelative, bool exponential, bool useTranslator)
        {
            if (segmentIndex >= mEnvelopeGenerator->getEnvelope().size())
                return;
            
            auto& segment = mEnvelopeGenerator->getEnvelope()[segmentIndex];
            segment.mDuration = duration;
            segment.mDestination = destination;
            segment.mDurationRelative = durationRelative;
            segment.mMode = exponential ? RampMode::Exponential : RampMode::Linear;
            segment.mTranslate = useTranslator;
        }

    }

}
