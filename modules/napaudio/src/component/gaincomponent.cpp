#include "gaincomponent.h"

// Nap includes
#include <nap/entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::GainComponent)
    RTTI_PROPERTY("Gain", &nap::audio::GainComponent::mGain, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Inputs", &nap::audio::GainComponent::mInputs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GainComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool GainComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            GainComponent* resource = rtti_cast<GainComponent>(getComponent());
            
            if (resource->mInputs.empty())
            {
                errorState.fail("%s: no inputs for gain", resource->mID.c_str());
                return false;
            }
            
            auto& nodeManager = getNodeManager();
            
            for (auto channel = 0; channel < resource->mInputs[0]->getChannelCount(); ++channel)
            {
                mGains.emplace_back(std::make_unique<Gain>(nodeManager));
                mGains[channel]->setGain(resource->mGain[channel % resource->mGain.size()]);
            }
            
            for (auto& input : resource->mInputs)
                for (auto channel = 0; channel < mGains.size(); ++channel)
                    mGains[channel]->inputs.connect(input->getOutputForChannel(channel % input->getChannelCount()));
            
            return true;
        }
        
    }
    
}
