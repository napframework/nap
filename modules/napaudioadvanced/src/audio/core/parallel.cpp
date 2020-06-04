#include "parallel.h"


// Nap includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// Audio includes
#include <audio/service/audioservice.h>


namespace nap
{
    
    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> Parallel::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            if (mChannels.empty())
            {
                errorState.fail("Channels in parallel may not be empty: %s", mID.c_str());
                return nullptr;
            }

            auto instance = std::make_unique<ParallelInstance>();
            for (auto channel = 0; channel < mChannelCount; ++channel)
            {
                if (!instance->addChannel(*mChannels[channel % mChannels.size()], nodeManager, errorState))
                {
                    errorState.fail("Failed to add channel %s to parallel %s", mChannels[channel % mChannels.size()]->mID.c_str(), mID.c_str());
                    return nullptr;
                }

            }

            return std::move(instance);
        }


        bool ParallelInstance::addChannel(AudioObject& channel, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto channelInstance = channel.instantiate<AudioObjectInstance>(nodeManager, errorState);
            if (channelInstance == nullptr)
            {
                errorState.fail("Failed to instantiate channel %s for ParallelInstance %s", channel.mID.c_str(), getName().c_str());
                return false;
            }
            if (channelInstance->getChannelCount() != 1)
            {
                errorState.fail("Channel %s for ParallelInstance %s is not mono", channel.mID.c_str(), getName().c_str());
                return false;
            }
            mChannels.emplace_back(std::move(channelInstance));

            return true;
        }


        void ParallelInstance::addChannel(std::unique_ptr<AudioObjectInstance> channel)
        {
            assert(channel->getChannelCount() == 1);
            mChannels.emplace_back(std::move(channel));
        }


        AudioObjectInstance* ParallelInstance::getChannelNonTyped(int channel)
        {
            if (channel < getChannelCount())
                return mChannels[channel].get();
            else
                return nullptr;
        }

    }
    
}
