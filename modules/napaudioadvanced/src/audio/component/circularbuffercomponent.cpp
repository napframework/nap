#include "circularbuffercomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::CircularBufferComponent)
    RTTI_PROPERTY("Input", &nap::audio::CircularBufferComponent::mInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Routing", &nap::audio::CircularBufferComponent::mChannelRouting, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::CircularBufferComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getChannel", &nap::audio::CircularBufferComponentInstance::getChannel)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool CircularBufferComponentInstance::init(utility::ErrorState& errorState)
        {
            auto resource = getComponent<CircularBufferComponent>();
            
            if (resource->mInput == nullptr)
            {
                errorState.fail("%s: Input not specified", resource->mID.c_str());
                return false;
            }
            
            auto audioService = getEntityInstance()->getCore()->getService<AudioService>();
            auto& nodeManager = audioService->getNodeManager();
            
            auto channelCount = resource->mChannelRouting.size();
            
            for (auto channel = 0; channel < channelCount; ++channel)
                if (resource->mChannelRouting[channel] >= mInput->getChannelCount())
                {
                    errorState.fail("%s: Trying to rout input channel that is out of bounds.", resource->mID.c_str());
                    return false;
                }
            
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                if (resource->mChannelRouting[channel] < 0)
                {
                    errorState.fail("%s: Trying to rout negative channel number.", resource->mID.c_str());
                    return false;
                }
                
                auto node = nodeManager.makeSafe<CircularBufferNode>(nodeManager, resource->mBufferSize);
                node->audioInput.connect(*mInput->getOutputForChannel(resource->mChannelRouting[channel]));
                mNodes.emplace_back(std::move(node));
            }
            return true;
        }
        
        
        CircularBufferNode* CircularBufferComponentInstance::getChannel(unsigned int channel)
        {
            if (channel >= mNodes.size())
                return nullptr;
            return mNodes[channel].getRaw();
        }

        
    }
        
}
