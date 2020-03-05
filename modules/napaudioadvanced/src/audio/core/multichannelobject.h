#pragma once

// Nap includes
#include <rtti/factory.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/nodeobject.h>

namespace nap
{
    
    namespace audio
    {

        class NAPAPI MultiChannelObjectInstanceBase : public AudioObjectInstance {
        RTTI_ENABLE(AudioObjectInstance)

        public:
            MultiChannelObjectInstanceBase() = default;
            MultiChannelObjectInstanceBase(const std::string& name) : AudioObjectInstance(name) { }

            virtual AudioObjectInstance* getChannelNonTyped(int channel) = 0;
        };


        template <typename T>
        class NAPAPI MultiChannelObjectInstance : public MultiChannelObjectInstanceBase {
        RTTI_ENABLE(MultiChannelObjectInstanceBase)

        public:
            MultiChannelObjectInstance() = default;
            MultiChannelObjectInstance(const std::string& name) : MultiChannelObjectInstanceBase(name) { }

            /**
             * Init contents by instancing each channel from a resource.
             * @param channelResource AudioObject resource to instantiate the channels from.
             * @param channelCount Number of channels to instantiate.
             * @param service Audio service for use to construct DSP
             * @param errorState Error state if the init fails
             * @return true on success
             */
            bool init(audio::AudioObject& channelResource, int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState)
            {
                for (auto channel = 0; channel < channelCount; ++channel)
                {
                    auto channelInstance = channelResource.instantiate<T>(nodeManager, errorState);
                    if (channelInstance == nullptr)
                    {
                        errorState.fail("Failed to instantiate channel %s for %s", channelResource.mID.c_str(), getName().c_str());
                        return false;
                    }
                    mChannels.emplace_back(std::move(channelInstance));
                }

                return true;
            }

            void addChannel(std::unique_ptr<T> channel) { mChannels.emplace_back(std::move(channel)); }

            // Inherited from AudioObjectInstance
            audio::OutputPin* getOutputForChannel(int channel) override { return mChannels[channel]->getOutputForChannel(0); }
            int getChannelCount() const override { return mChannels.size(); }
            void connect(unsigned int channel, audio::OutputPin& pin) override { mChannels[channel]->connect(0, pin); }
            int getInputChannelCount() const override { return (mChannels[0]->getInputChannelCount() == 1) ? mChannels.size() : 0; }

            T* getChannel(int channel) { return mChannels[channel].get(); }
            AudioObjectInstance* getChannelNonTyped(int channel) override { return mChannels[channel].get(); }

        private:
            std::vector<std::unique_ptr<T>> mChannels;
        };


    }
        
}


