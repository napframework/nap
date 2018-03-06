#include "voice.h"

// Audio includes
#include <audio/core/polyphonicobject.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::Voice)
    RTTI_CONSTRUCTOR(nap::audio::NodeManager&)
    RTTI_PROPERTY("Envelope", &nap::audio::Voice::mEnvelope, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::audio::VoiceInstance)
    RTTI_FUNCTION("play", &nap::audio::VoiceInstance::play)
    RTTI_FUNCTION("stop", &nap::audio::VoiceInstance::stop)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        bool VoiceInstance::init(Voice& resource, utility::ErrorState& errorState)
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
            
            mEnvelope->getEnvelopeFinishedSignal().connect(this, &VoiceInstance::envelopeFinished);
                        
            return true;
        }
        
        
        void VoiceInstance::play(TimeValue duration)
        {
            mEnvelope->trigger(duration);
            mStartTime = getResource().getNodeManager().getSampleTime();
        }
        
        
        void VoiceInstance::stop(TimeValue rampTime)
        {
            mEnvelope->stop(rampTime);
        }
        
        
        bool VoiceInstance::try_use()
        {
            bool expected = false;
            return (mBusy.compare_exchange_strong(expected, true));
        }
        
        
        void VoiceInstance::free()
        {
            bool expected = true;
            while (!mBusy.compare_exchange_weak(expected, false)) { }
        }

        
        void VoiceInstance::envelopeFinished(EnvelopeGenerator&)
        {
            finishedSignal(*this);
            mBusy = false;
        }


        
    }
    
}
