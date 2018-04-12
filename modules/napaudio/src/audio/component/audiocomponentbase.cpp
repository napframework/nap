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
        
        NodeManager& AudioComponentBaseInstance::getNodeManager()
        {
            return getAudioService().getNodeManager();
        }
        
        AudioService& AudioComponentBaseInstance::getAudioService()
        {
            return *getEntityInstance()->getCore()->getService<AudioService>(rtti::ETypeCheck::EXACT_MATCH);
        }

        
    }
    
}
