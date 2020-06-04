#pragma once

#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/core/nodeobject.h>

namespace nap
{

    namespace audio
    {

        /**
         * A node that manages a nested node manager. The nested node manager can contain a DSP network that runs on a lower buffersize than the main node system. This is useful for more time-accurate scheduling of events or parameter changes.
         * The @NestedNodeManager has its own internal buffersize, input channel count and output channel count. The sample rate should be the same as the main node manager's samplerate. A possible new feature in the future could be resampling so the nested node manager can run on a different samplerate than the main system as well.
         */
        class NestedNodeManagerNode : public Node
        {
        public:
            /**
             * Constructor
             * @param parentNodeManager the node manager that this NestedNodeManagerNode runs in. The parent node manager should run an internal buffersize that is a whole number of times the internal buffersize of the nested node manager.
             */
            NestedNodeManagerNode(NodeManager& parentNodeManager) : Node(parentNodeManager), mNestedNodeManager(parentNodeManager.getDeletionQueue())
            {

            }


            /**
             * Initialize the node.
             * @param inputChannelCount the numebr of input channels the nested node manager has
             * @param outputChannelCount  the number of output channels the nested node manager has
             * @param internalBufferSize the internal buffersize of the nested node manager. Should be smaller than the parent node manager's.
             */
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

            /**
             * Return input pin with given index that will be fed into the nested node system.
             */
            InputPin& getInput(int index) { return _mInputs[index]; }

            /**
             * Return output pin with given index that holds the output of the nested node system.
             */
            OutputPin& getOutput(int index) { return _mOutputs[index]; }

            /**
             * @return number of input channels of the nested node system.
             */
            int getInputCount() const { return _mInputs.size(); }

            /**
             *
             * @return number of output channels of the nested node system.
             */
            int getOutputCount() const { return _mOutputs.size(); }

            /**
             * @return the nested node manager managed by this node.
             */
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


        /**
         * AudioObject wrapping a nested node manager. See @NestedNodeManagerNode for mroe info.
         */
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