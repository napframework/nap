#include "audiobufferresource.h"

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioBufferResource)
    RTTI_CONSTRUCTOR(nap::audio::AudioService&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        AudioBufferResource::AudioBufferResource(AudioService& service)
        {
            mBuffer = service.getNodeManager().makeSafe<MultiSampleBuffer>();
        }

    }
    
}

