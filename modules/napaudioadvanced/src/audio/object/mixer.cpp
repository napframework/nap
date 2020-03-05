#include "mixer.h"

RTTI_BEGIN_CLASS(nap::audio::Mixer)
    RTTI_PROPERTY("Inputs", &nap::audio::Mixer::mInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChannelInstance<nap::audio::MixNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
    }
    
}
