#include "gain.h"

RTTI_BEGIN_CLASS(nap::audio::Gain)
    RTTI_PROPERTY("Gain", &nap::audio::Gain::mGain, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Inputs", &nap::audio::Gain::mInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::GainNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        bool Gain::initNode(int channel, GainNode& node, utility::ErrorState& errorState)
        {
            for (auto& input : mInputs)
                if (input != nullptr)
                {
                    node.inputs.connect(*input->getInstance()->getOutputForChannel(channel % input->getInstance()->getChannelCount()));
                }
            node.setGain(mGain[channel % mGain.size()], 0);

            return true;
        }
    }
    
}
