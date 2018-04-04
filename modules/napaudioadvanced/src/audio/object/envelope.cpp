#include "envelope.h"

RTTI_BEGIN_CLASS(nap::audio::Envelope)
    RTTI_PROPERTY("Envelope", &nap::audio::Envelope::mSegments, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("AutoTrigger", &nap::audio::Envelope::mAutoTrigger, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EnvelopeInstance)
    RTTI_FUNCTION("trigger", &nap::audio::EnvelopeInstance::trigger)
    RTTI_FUNCTION("stop", &nap::audio::EnvelopeInstance::stop)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
         
        std::unique_ptr<AudioObjectInstance> Envelope::createInstance()
        {
            return std::make_unique<EnvelopeInstance>(*this);
        }
        
        
        bool EnvelopeInstance::init(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mEnvelopeGenerator = make_node<EnvelopeGenerator>(nodeManager);
            auto resource = rtti_cast<Envelope>(&getResource());
            
            // We copy the envelope data from the resource to a shared pointer that can be safely shared with the node
            mSegments = std::make_shared<EnvelopeGenerator::Envelope>(resource->mSegments);
            if (resource->mAutoTrigger)
                mEnvelopeGenerator->trigger(mSegments);
            return true;
        }
        

    }
    
}
