#include "delaycomponent.h"

// Nap includes
#include <nap/entity.h>

// Audio includes
#include <node/audionodemanager.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::DelayComponent)
    RTTI_PROPERTY("Input", &nap::audio::DelayComponent::mInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Time", &nap::audio::DelayComponent::mTime, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DryWet", &nap::audio::DelayComponent::mDryWet, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Feedback", &nap::audio::DelayComponent::mFeedback, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Routing", &nap::audio::DelayComponent::mInputRouting, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::DelayComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap {
    
    namespace audio {
    
        bool DelayComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            DelayComponent* resource = rtti_cast<DelayComponent>(getComponent());
            
            AudioComponentInstance* input = rtti_cast<AudioComponentInstance>(resource->mInput.get());
            if (!input)
            {
                errorState.fail("%s: Input is not an audio component", resource->mID.c_str());
                return false;
            }
            
            auto& nodeManager = getNodeManager();
            
            for (auto channel = 0; channel < resource->mInputRouting.size(); ++channel)
            {
                if (resource->mInputRouting[channel] >= input->getChannelCount())
                {
                    errorState.fail("%s: Trying to rout channel that is out of bounds.", resource->mID.c_str());
                    return false;
                }
                
                if (resource->mTime.empty())
                {
                    errorState.fail("%s Delaytime values is empty array", resource->mID.c_str());
                    return false;
                }
                
                mDelays.emplace_back(std::make_unique<DelayNode>(nodeManager));                
                mDelays[channel]->setTime(resource->mTime[channel % resource->mTime.size()] * nodeManager.getSamplesPerMillisecond());
                mDelays[channel]->setDryWet(resource->mDryWet);
                mDelays[channel]->setFeedback(resource->mFeedback);
                mDelays[channel]->input.connect(input->getOutputForChannel(resource->mInputRouting[channel]));
            }
            
            return true;
        }
        
    }
    
}
