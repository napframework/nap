#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audioobject.h>


namespace nap
{
    
    namespace audio
    {


        template <typename NodeType> class NodeObjectInstance;


        template <typename NodeType>
        class NodeObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            NodeObject() : AudioObject() { }
            
        private:
            virtual void initNode(NodeType& node) { }

            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override
            {
                auto instance = std::make_unique<NodeObjectInstance<NodeType>>(nodeManager);
                instance->init(nodeManager, errorState);
                initNode(*instance->get());
                return std::move(instance);
            }
        };


        class NodeObjectInstanceBase : public AudioObjectInstance {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            NodeObjectInstanceBase() = default;
            NodeObjectInstanceBase(const std::string& name) : AudioObjectInstance(name) { }

            virtual Node* getNonTyped() = 0;
        };
        

        template <typename NodeType>
        class NodeObjectInstance : public NodeObjectInstanceBase
        {
            RTTI_ENABLE(NodeObjectInstanceBase)
            
        public:
            NodeObjectInstance() = default;

            NodeObjectInstance(const std::string& name) : NodeObjectInstanceBase(name) { }

            bool init(NodeManager& nodeManager, utility::ErrorState& errorState)
            {
                mNode = nodeManager.makeSafe<NodeType>(nodeManager);
                return true;
            }

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override { return mNode->getOutputs().size();; }
            void connect(unsigned int channel, OutputPin& pin) override;
            int getInputChannelCount() const override { return mNode->getInputs().size(); }

            SafePtr<NodeType> get() { return mNode.get(); }
            NodeType* getRaw() { return mNode.getRaw(); }
            Node* getNonTyped() override { return mNode.getRaw(); }

        private:
            SafeOwner<NodeType> mNode = nullptr;
        };


        class NAPAPI MultiChannelBase : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            MultiChannelBase() = default;
            int mChannelCount = 1; ///< Property: 'ChannelCount' The number of channels
        };


        template <typename NodeType>
        class MultiChannel : public MultiChannelBase
        {
            RTTI_ENABLE(MultiChannelBase)

        public:
            MultiChannel() = default;

            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;

        private:
            virtual bool initNode(int channel, NodeType& node, utility::ErrorState& errorState) { return true; }
        };


        class MultiChannelInstanceBase : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)
        public:
            virtual Node* getChannelNonTyped(int channel) = 0;
        };


        template <typename NodeType>
        class MultiChannelInstance : public MultiChannelInstanceBase
        {
            RTTI_ENABLE(MultiChannelInstanceBase)

        public:
            using NodeCreationFunction = std::function<std::unique_ptr<NodeType>()>;

        public:
            MultiChannelInstance() = default;

            // Inherited from MultiChannelInstanceBase
            Node* getChannelNonTyped(int channel) override { return channel < mChannels.size() ? mChannels[channel].getRaw() : nullptr; }

            bool init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState);

            /**
             * Returns a pointer to the DSP node for the specified channel.
			 * Returns nullptr if the channel; is out of bounds.
             */
            NodeType* getChannel(unsigned int channel) { return channel < mChannels.size() ? mChannels[channel].getRaw() : nullptr; }

            /**
             * Clear the processing channels.
             */
            void clear() { mChannels.clear(); }

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return *mChannels[channel]->getOutputs().begin(); }
            int getChannelCount() const override { return mChannels.size(); }
            void connect(unsigned int channel, OutputPin& pin) override { (*mChannels[channel]->getInputs().begin())->connect(pin); }
            int getInputChannelCount() const override { return (mChannels[0]->getInputs().size() == 1) ? mChannels.size() : 0; }

        private:
            std::vector<SafeOwner<NodeType>> mChannels;
        };


        template <typename NodeType>
        OutputPin* NodeObjectInstance<NodeType>::getOutputForChannel(int channel)
        {
            auto i = 0;
            for (auto& output : mNode->getOutputs())
            {
                if (i == channel)
                    return output;
                i++;
            }
            return nullptr;
        }


        template <typename NodeType>
        void NodeObjectInstance<NodeType>::connect(unsigned int channel, OutputPin& pin)
        {
            auto i = 0;
            for (auto& input : mNode->getInputs())
            {
                if (i == channel)
                {
                    input->connect(pin);
                    return;
                }
                i++;
            }
        }


        template <typename NodeType>
        std::unique_ptr<AudioObjectInstance> MultiChannel<NodeType>::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<MultiChannelInstance<NodeType>>();
            if (!instance->init(mChannelCount, nodeManager, errorState))
                return nullptr;
            for (auto channel = 0; channel < instance->getChannelCount(); ++channel)
                if (!initNode(channel, *instance->getChannel(channel), errorState))
                {
                    errorState.fail("Failed to init node at channel %i", channel);
                    return nullptr;
                }

            return std::move(instance);
        }


        template <typename NodeType>
        bool MultiChannelInstance<NodeType>::init(int channelCount, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            for (auto channel = 0; channel < channelCount; ++channel)
            {
                auto node = nodeManager.makeSafe<NodeType>(nodeManager);

                if (node == nullptr)
                {
                    errorState.fail("Failed to create node.");
                    return false;
                }

                mChannels.emplace_back(std::move(node));
            }

            return true;
        }


    }
    
}

