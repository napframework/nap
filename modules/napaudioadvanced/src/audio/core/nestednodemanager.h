#pragma once

#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/core/nodeobject.h>

namespace nap
{

    namespace audio
    {

        class NestedNodeManagerNode : public Node
        {
        public:
            NestedNodeManagerNode(NodeManager& parentNodeManager) : Node(parentNodeManager), mNestedNodeManager(parentNodeManager.getDeletionQueue())
            {

            }

            void init(int inputChannelCount, int outputChannelCount, int internalBufferSize)
            {
                mNestedNodeManager.setInputChannelCount(inputChannelCount);
                mNestedNodeManager.setOutputChannelCount(outputChannelCount);
                mNestedNodeManager.setSampleRate(getNodeManager().getSampleRate());
                mNestedNodeManager.setInternalBufferSize(internalBufferSize);

                for (auto i = 0; i < outputChannelCount; ++i)
                {
                    _mOutputs.emplace_back(OutputPin(this));
                    mOutputBuffers.emplace_back(nullptr);
                }
                for (auto i = 0; i < inputChannelCount; ++i)
                {
                    _mInputs.emplace_back(InputPin(this));
                    mInputBuffers.emplace_back(nullptr);
                }
            }

            InputPin& getInput(int index) { return _mInputs[index]; }
            OutputPin& getOutput(int index) { return _mOutputs[index]; }
            int getInputCount() const { return _mInputs.size(); }
            int getOutputCount() const { return _mOutputs.size(); }

            NodeManager& getNestedNodeManager() { return mNestedNodeManager; }

        private:
            void process() override
            {
                for (auto i = 0; i < _mInputs.size(); ++i)
                {
                    auto inputBuffer = _mInputs[i].pull();
                    if (inputBuffer == nullptr)
                        mInputBuffers[i] = nullptr;
                    else
                        mInputBuffers[i] = inputBuffer;
                }

                for (auto i = 0; i < _mOutputs.size(); ++i)
                {
                    auto outputBuffer = &getOutputBuffer(_mOutputs[i]);
                    mOutputBuffers[i] = outputBuffer;
                }
                mNestedNodeManager.process(mInputBuffers, mOutputBuffers, getBufferSize());
            }

            NodeManager mNestedNodeManager;
            std::vector<InputPin> _mInputs;
            std::vector<OutputPin> _mOutputs;
            std::vector<audio::SampleBuffer*> mOutputBuffers;
            std::vector<audio::SampleBuffer*> mInputBuffers;
        };


        class NestedNodeManagerInstance : public AudioObjectInstance
        {
        public:
            NestedNodeManagerInstance() = default;
            NestedNodeManagerInstance(const std::string& name) : AudioObjectInstance(name) { }

            bool init(NodeManager& nodeManager, int inputChannelCount, int outputChannelCount, int internalBufferSize, utility::ErrorState& errorState)
            {
                mNode = nodeManager.makeSafe<NestedNodeManagerNode>(nodeManager);
                mNode->init(inputChannelCount, outputChannelCount, internalBufferSize);
                return true;
            }

            // Inherited from AudioObjectInstance
            OutputPin* getOutputForChannel(int channel) override { return &mNode->getOutput(channel); }
            int getChannelCount() const override { return mNode->getOutputCount(); }
            void connect(unsigned int channel, OutputPin& pin) override { mNode->getInput(channel).connect(pin); }
            int getInputChannelCount() const override { return mNode->getInputCount(); }

            NodeManager& getNestedNodeManager() { return mNode->getNestedNodeManager(); }
            Process& getProcess() { return *mNode; }

        private:
            SafeOwner<NestedNodeManagerNode> mNode = nullptr;
        };

    }

}