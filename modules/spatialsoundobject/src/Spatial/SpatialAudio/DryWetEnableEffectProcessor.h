#pragma once

// Spatial includes
#include <Spatial/SpatialAudio/Effect.h>
#include <Spatial/Audio/DryWetEnable.h> // inside ParallelNodeObjectInstance

// Audio includes
#include <audio/core/parallel.h>
#include <audio/service/audioservice.h>

namespace nap
{

    namespace spatial
    {
        
        /**
         * EffectProcessor with dry/wet and enable functionality included.
         */
        class NAPAPI DryWetEnableEffectProcessor : public EffectProcessor
        {
            RTTI_ENABLE(EffectProcessor)
            
        public:
            DryWetEnableEffectProcessor(EffectBase& effect) : EffectProcessor(effect) { }

            // Inherited from EffectProcessor
            bool init(audio::AudioService& audioService, int channelCount, utility::ErrorState& errorState) override;
            int getChannelCount() const override { return mChannelCount; }
            audio::OutputPin* getOutputForChannel(int channel) override { return mDryWetInstance->getOutputForChannel(channel); }
            int getInputChannelCount() const override { return mChannelCount; }
            void connect(unsigned int channel, audio::OutputPin& pin) override;

            /**
             * Should create and initialize the custom DSP for this processor.
             */
            virtual std::unique_ptr<audio::AudioObjectInstance> createDSP(audio::AudioService& audioService, int channelCount, utility::ErrorState& errorState) = 0;

            /**
             * Enables/disables the processor by bypassing the processing.
             */
            void setEnable(bool value);

            /**
             * Adjusts the dry/wet balance of the processor.
             */
            void setDryWet(float value);

            /**
             * Separately sets the dry level of the processor.
             */
            void setDry(float value);
            
            /**
             * Separately sets the wet level of the processor.
             */
            void setWet(float value);
            
            
            /**
             * Enables/disables a single channel of the processor.
             */
            void setEnable(bool value, int channel);
            
            /**
             * Adjusts the dry/wet balance of a single channel of the processor.
             */
            void setDryWet(float value, int channel);
            
            /**
             * Separately sets the dry level of a single channel of the processor.
             */
            void setDry(float value, int channel);
            
            /**
             * Separately sets the wet level of a signel channel of the processor.
             */
            void setWet(float value, int channel);
            
            
        protected:
            /**
             * This function can optionally be overwritten to define custom connection behaviour of input into this processor.
             * @param channel Channel to connect to.
             * @param pin Pin to be connected to this processor's input.
             */
            virtual void connectToDSP(unsigned int channel, audio::OutputPin& pin) { mDSP->connect(channel, pin); }
            
        private:
            int mChannelCount = 0;
            std::unique_ptr<audio::AudioObjectInstance> mDSP = nullptr;
            std::unique_ptr<audio::ParallelNodeObjectInstance<audio::DryWetEnableNode>> mDryWetInstance = nullptr;
        };



        /**
         * Templated class that defines an EffectProcessor with a mono audioobject per channel.
         * InstanceType is an AudioObjectInstance descendant that will contain custom processing for one single channel.
         */
        template <typename InstanceType>
        class NAPAPI ParallelMonoEffectProcessor : public DryWetEnableEffectProcessor
        {
            RTTI_ENABLE(DryWetEnableEffectProcessor)
            
        public:
            using DryWetEnableEffectProcessor::DryWetEnableEffectProcessor;

            // Inherited from @DryWetEnableEffectProcessor
            std::unique_ptr<audio::AudioObjectInstance> createDSP(audio::AudioService& audioService, int channelCount, utility::ErrorState& errorState) override
            {
                auto dspInstance = std::make_unique<audio::ParallelInstance>();
                for (auto channel = 0; channel < channelCount; ++channel)
                {
                    auto channelInstance = std::make_unique<InstanceType>();
                    if (!initDSPChannel(channel, *channelInstance, audioService, errorState))
                    {
                        errorState.fail("Failed to initialize channel in ParallelMonoEffectProcessor");
                        return nullptr;
                    }
                    dspInstance->addChannel(std::move(channelInstance));
                }
                mDSPInstance = dspInstance.get();
                return std::move(dspInstance);
            }
            
            /**
             * Should initialise the custom DSP 'channelInstance' for the given channel 'channel'.
             * initDSPChannel() will be called at initialisation for every channelInstance.
             */
            virtual bool initDSPChannel(int channel, InstanceType& channelInstance, audio::AudioService& audioService, utility::ErrorState& errorState) = 0;

            /**
             * Returns the AudioObjectInstance of InstanceType that performs processing for @channel.
             */
            InstanceType* getDSP(int channel) { return mDSPInstance->getChannel<InstanceType>(channel); }

        private:
            audio::ParallelInstance* mDSPInstance = nullptr;
        };

        
        /**
         * @ParallelNodeEffectProcessor is a parallelMonoEffectProcessor with type NodeObject.
         * It allows you to easily create an effect processor from a single mono node.
         */
        template <typename NodeType>
        class NAPAPI ParallelNodeEffectProcessor : public ParallelMonoEffectProcessor<audio::NodeObjectInstance<NodeType>> {
        public:
            using ParallelMonoEffectProcessor<audio::NodeObjectInstance<NodeType>>::ParallelMonoEffectProcessor;

            bool initDSPChannel(int channel, audio::NodeObjectInstance<NodeType>& channelInstance, audio::AudioService& audioService, utility::ErrorState& errorState) override
            {
                if (!channelInstance.init(audioService.getNodeManager(), errorState))
                    return false;
                setChannelDefaults(channel, *channelInstance.get());
                return true;
            }
            
            audio::SafePtr<NodeType> getNode(int channel){
                return ParallelMonoEffectProcessor<audio::NodeObjectInstance<NodeType>>::getDSP(channel)->get();
            }

        protected:
            /**
             * Can be implemented in order to override the default settings of the nodes.
             * setChannelDefaults() will be called at initialisation for every channelInstance.
             */
            virtual void setChannelDefaults(int channel, NodeType& channelInstance) { }
        };
        

    }
}
