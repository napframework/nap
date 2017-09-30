#include "envelope.h"

RTTI_BEGIN_CLASS(nap::audio::Envelope)
    RTTI_PROPERTY("Envelope", &nap::audio::Envelope::mSegments, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
         
        std::unique_ptr<AudioObjectInstance> Envelope::createInstance()
        {
            return std::make_unique<EnvelopeInstance>(*this);
        }
    }
    
}
