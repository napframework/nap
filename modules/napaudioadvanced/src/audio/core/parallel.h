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

        class NAPAPI Parallel : public AudioObject {
            RTTI_ENABLE(AudioObject)
        public:
            std::vector<ResourcePtr<AudioObject>> mChannels;
            ResourcePtr<AudioObject> mInput = nullptr;
            int mChannelCount = 1;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        class NAPAPI ParallelInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            ParallelInstance() = default;
            ParallelInstance(const std::string& name) : AudioObjectInstance(name) { }

            bool addChannel(AudioObject& resource, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Add a mono audio object instance as a new channel of the object
             */
            void addChannel(std::unique_ptr<AudioObjectInstance> channel);

            /**
             * Returns a channel as mono audio object
             */
            template <typename T>
            T* getChannel(int channel) { return rtti_cast<T>(mChannels[channel].get()); }

            AudioObjectInstance* getChannelNonTyped(int channel);

            // Inherited from AudioObjectInstance
            audio::OutputPin* getOutputForChannel(int channel) override { return mChannels[channel]->getOutputForChannel(0); }
            int getChannelCount() const override { return mChannels.size(); }
            void connect(unsigned int channel, audio::OutputPin& pin) override { mChannels[channel]->connect(0, pin); }
            int getInputChannelCount() const override { return (mChannels[0]->getInputChannelCount() == 1) ? mChannels.size() : 0; }

        protected:
            std::vector<std::unique_ptr<AudioObjectInstance>> mChannels;

        };


    }
        
}


