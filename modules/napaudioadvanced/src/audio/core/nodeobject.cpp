#include "nodeobject.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::NodeObjectInstanceBase)
        RTTI_FUNCTION("get", &nap::audio::NodeObjectInstanceBase::getNonTyped)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChannelBase)
    RTTI_PROPERTY("ChannelCount", &nap::audio::MultiChannelBase::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::MultiChannelInstanceBase)
    RTTI_FUNCTION("getChannel", &nap::audio::MultiChannelInstanceBase::getChannelNonTyped)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {



    }
    
}
