#include "audiocomponent.h"


// Nap includes
#include <nap/entity.h>
#include <nap/core.h>

// Audio includes
#include <service/audioservice.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioComponent)
    RTTI_PROPERTY("Object", &nap::audio::AudioComponent::mObject, nap::rtti::EPropertyMetaData::Embedded)
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
            mObject = std::move(resource->mObject->instantiate(getNodeManager(), errorState));
            if (!mObject)
                return false;
            
            return true;
        }
        
        
        AudioObjectInstance* AudioComponentInstance::getObject()
        {
            return mObject.get();
        }

    
        NodeManager& AudioComponentInstance::getNodeManager()
        {
            return getEntityInstance()->getCore()->getService<AudioService>(ETypeCheck::IS_DERIVED_FROM)->getNodeManager();
        }
        
    }
    
}
