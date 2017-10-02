#include "voicegraph.h"

RTTI_BEGIN_CLASS(nap::audio::VoiceGraph)
    RTTI_PROPERTY("Envelope", &nap::audio::VoiceGraph::mEnvelope, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::VoiceGraphInstance)
    RTTI_FUNCTION("play", &nap::audio::VoiceGraphInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::VoiceGraphInstance::stop)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
        
        bool VoiceGraphInstance::init(VoiceGraph& resource, utility::ErrorState& errorState)
        {
            if (!GraphInstance::init(resource, errorState))
                return false;

            for (auto& object : getObjects())
                if (&object->getResource() == resource.mEnvelope.get())
                    mEnvelope = rtti_cast<EnvelopeInstance>(object.get());

            if (mEnvelope == nullptr)
            {
                errorState.fail("%s envelope not found", resource.mID.c_str());
                return false;
            }
            
            mEnvelope->getEnvelopeFinishedSignal().connect(this, &VoiceGraphInstance::envelopeFinished);
                        
            return true;
        }
        
        
        void VoiceGraphInstance::play(TimeValue duration)
        {
            auto resource = rtti_cast<VoiceGraph>(&getResource());
            mEnvelope->trigger(duration);
            mStartTime = getResource().getNodeManager().getSampleTime();
        }
        
        
        void VoiceGraphInstance::stop(TimeValue rampTime)
        {
            mEnvelope->stop(rampTime);
        }
        
        
        void VoiceGraphInstance::envelopeFinished(EnvelopeGenerator&)
        {
            finishedSignal(*this);
            mBusy = false;
        }


        
    }
    
}
