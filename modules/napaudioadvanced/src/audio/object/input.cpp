#include "input.h"


RTTI_BEGIN_CLASS(nap::audio::Input)
    RTTI_PROPERTY("Channels", &nap::audio::Input::mChannels, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChannelInstance<nap::audio::InputNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
    }
    
}
