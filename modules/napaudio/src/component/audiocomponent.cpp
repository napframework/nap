#include "audiocomponent.h"


// Nap includes
#include <nap/entity.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioComponent)
    RTTI_PROPERTY("AudioInterface", &nap::audio::AudioComponent::mAudioInterface, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
    
    }
    
}
