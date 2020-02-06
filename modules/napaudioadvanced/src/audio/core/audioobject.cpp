#include "audioobject.h"


// Nap includes
#include <nap/logger.h>



// RTTI
RTTI_DEFINE_BASE(nap::audio::AudioObject)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioObjectInstance)
    RTTI_FUNCTION("getChannelCount", &nap::audio::AudioObjectInstance::getChannelCount)
    RTTI_FUNCTION("getOutputForChannel", &nap::audio::AudioObjectInstance::tryGetOutputForChannel)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
                
    }
    
}
