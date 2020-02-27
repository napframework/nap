#include "audiocomponent.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioComponent)
    RTTI_PROPERTY("Object", &nap::audio::AudioComponent::mObject, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Links", &nap::audio::AudioComponent::mLinks, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getObject", &nap::audio::AudioComponentInstance::getObject)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        bool AudioComponentInstance::init(utility::ErrorState& errorState)
        {
            AudioComponent* resource = getComponent<AudioComponent>();
            mObject = std::move(resource->mObject->instantiate<AudioObjectInstance>(getAudioService().getNodeManager(), errorState));
            if (!mObject)
                return false;
            
            return true;
        }
        
        
        AudioObjectInstance* AudioComponentInstance::getObject()
        {
            return mObject.get();
        }

    
    }
    
}
