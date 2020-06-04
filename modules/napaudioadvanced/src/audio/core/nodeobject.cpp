#include "nodeobject.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::NodeObjectInstanceBase)
        RTTI_FUNCTION("get", &nap::audio::NodeObjectInstanceBase::getNonTyped)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectBase)
    RTTI_PROPERTY("ChannelCount", &nap::audio::ParallelNodeObjectBase::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstanceBase)
    RTTI_FUNCTION("getChannel", &nap::audio::ParallelNodeObjectInstanceBase::getChannelNonTyped)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {



    }
    
}
