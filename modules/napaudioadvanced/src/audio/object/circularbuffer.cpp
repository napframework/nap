#include "circularbuffer.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::CircularBuffer)
    RTTI_PROPERTY("Input", &nap::audio::CircularBuffer::mInput, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Routing", &nap::audio::CircularBuffer::mChannelRouting, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("BufferSize", &nap::audio::CircularBuffer::mBufferSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("RootProcess", &nap::audio::CircularBuffer::mRootProcess, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::CircularBufferInstance)
    RTTI_FUNCTION("getChannel", &nap::audio::CircularBufferInstance::getChannel)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> CircularBuffer::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            if (mInput == nullptr)
            {
                errorState.fail("%s: Input not specified", mID.c_str());
                return nullptr;
            }
            
            auto instance = std::make_unique<CircularBufferInstance>();
            if (!instance->init(*mInput->getInstance(), mChannelRouting, mRootProcess, mBufferSize, nodeManager, errorState))
                return nullptr;
            
            return std::move(instance);
        }

    
        bool CircularBufferInstance::init(AudioObjectInstance& input, const std::vector<int>& channelRouting, bool rootProcess, int bufferSize, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto channelCount = channelRouting.size();
            
            for (auto channel = 0; channel < channelCount; ++channel)
                if (channelRouting[channel] >= input.getChannelCount())
                {
                    errorState.fail("%s: Trying to rout input channel that is out of bounds.", getName().c_str());
                    return false;
                }
            
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                if (channelRouting[channel] < 0)
                {
                    errorState.fail("%s: Trying to rout negative channel number.", getName().c_str());
                    return false;
                }
                
                auto node = nodeManager.makeSafe<CircularBufferNode>(nodeManager, bufferSize, rootProcess);
                node->audioInput.connect(*input.getOutputForChannel(channelRouting[channel]));
                mNodes.emplace_back(std::move(node));
            }
            return true;
        }


        bool CircularBufferInstance::init(int channelCount, bool rootProcess, int bufferSize, NodeManager &nodeManager,
                                          utility::ErrorState &errorState) {
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                auto node = nodeManager.makeSafe<CircularBufferNode>(nodeManager, bufferSize, rootProcess);
                mNodes.emplace_back(std::move(node));
            }

            return true;
        }


        SafePtr<CircularBufferNode> CircularBufferInstance::getChannel(unsigned int channel)
        {
            if (channel >= mNodes.size())
                return nullptr;
            return mNodes[channel].get();
        }


    }
        
}
