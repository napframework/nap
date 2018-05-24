#include "audiocomponentbase.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioComponentBase)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioComponentBaseInstance)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        AudioComponentBaseInstance::AudioComponentBaseInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource)
        {
            mAudioService = entity.getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH);
        }
        
        
        NodeManager& AudioComponentBaseInstance::getNodeManager()
        {
            return mAudioService->getNodeManager();
        }

        
    }
    
}
