#include "delay.h"

RTTI_BEGIN_CLASS(nap::audio::DelayObject)
    RTTI_PROPERTY("Time", &nap::audio::DelayObject::mTime, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Feedback", &nap::audio::DelayObject::mFeedback, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DryWet", &nap::audio::DelayObject::mDryWet, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Input", &nap::audio::DelayObject::mInput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::DelayNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool DelayObject::initNode(int channel, DelayNode& node, utility::ErrorState& errorState)
        {
            node.setTime(mTime[channel % mTime.size()]);
            node.setFeedback(mFeedback[channel % mFeedback.size()]);
            node.setDryWet(mDryWet[channel % mDryWet.size()]);
            if (mInput != nullptr)
                node.input.connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));

            return true;
        }
    }
    
}
