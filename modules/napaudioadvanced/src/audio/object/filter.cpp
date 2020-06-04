#include "filter.h"

RTTI_BEGIN_CLASS(nap::audio::Filter)
    RTTI_PROPERTY("Mode", &nap::audio::Filter::mMode, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Frequency", &nap::audio::Filter::mFrequency, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Resonance", &nap::audio::Filter::mResonance, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Band", &nap::audio::Filter::mBand, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Gain", &nap::audio::Filter::mGain, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::FilterNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool Filter::initNode(int channel, FilterNode& node, utility::ErrorState& errorState)
        {
            node.setMode(mMode);
            node.setFrequency(mFrequency[channel % mFrequency.size()]);
            node.setResonance(mResonance[channel % mResonance.size()]);
            node.setBand(mBand[channel % mBand.size()]);
            node.setGain(mGain[channel % mGain.size()]);
            if (mInput != nullptr)
                node.audioInput.connect(*mInput->getInstance()->getOutputForChannel(channel % mInput->getInstance()->getChannelCount()));
            return true;
        }
    }
    
}
