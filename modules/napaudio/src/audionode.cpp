#include "audionode.h"
#include "audionodemanager.h"


namespace nap {
    
    namespace audio {
        
        
        InputPin::~InputPin()
        {
            disconnect();
        }
        
        
        SampleBufferPtr InputPin::pull()
        {
            if (mInput)
                return mInput->pull();
            else
                return nullptr;
        }
        
        
        void InputPin::connect(OutputPin& input)
        {
            // disconnect any existing connection
            disconnect();
            
            // make the input and output point to one another
            mInput = &input;
            input.mOutputs.emplace(this);
        }
        
        
        void InputPin::disconnect()
        {
            if (mInput)
            {
                mInput->mOutputs.erase(this);
                mInput = nullptr;
            }
        }

        
        OutputPin::OutputPin(Node* node)
        {
            node->mOutputs.emplace(this);
            mNode = node;
            setBufferSize(mNode->getBufferSize());
        }

        
        OutputPin::~OutputPin()
        {
            mNode->mOutputs.erase(this);
            disconnectAll();
        }
        
        
        void OutputPin::disconnectAll()
        {
            for (InputPin* output : mOutputs)
            {
                assert(output->mInput == this);
                output->mInput = nullptr;
            }
            mOutputs.clear();            
        }
        
        
        SampleBufferPtr OutputPin::pull()
        {
            mNode->update();
            return &mBuffer;
        }
        
        
        void OutputPin::setBufferSize(int bufferSize)
        {
            mBuffer.resize(bufferSize);
        }
        
        
        Node::Node(NodeManager& service)
        {
            mNodeManager = &service;
            service.registerNode(*this);
        }
        
        
        Node::~Node()
        {
            mNodeManager->unregisterNode(*this);
        }
        
        
        int Node::getBufferSize() const
        {
            return mNodeManager->getInternalBufferSize();
        }
        
        
        float Node::getSampleRate() const
        {
            return mNodeManager->getSampleRate();
        }
        
        
        DiscreteTimeValue Node::getSampleTime() const
        {
            return mNodeManager->getSampleTime();
        }
        
        
        void Node::update()
        {
            if (mLastCalculatedSample < getSampleTime())
            {
                process();
                mLastCalculatedSample = getSampleTime();
            }
        }
        
        
        void Node::setBufferSize(int bufferSize)
        {
            for (auto& output : mOutputs)
                output->setBufferSize(bufferSize);
            
            bufferSizeChanged(bufferSize);
        }
        
        
        OutputNode::OutputNode(NodeManager& manager, bool active) : Node(manager)
        {
            manager.registerRootNode(*this);
            mActive = active;
        }
        
        
        OutputNode::~OutputNode()
        {
            mNodeManager->unregisterRootNode(*this);
        }
        
        
        void OutputNode::process()
        {
            if (!mActive)
                return;
            
            SampleBufferPtr buffer = audioInput.pull();
            if (buffer)
                mNodeManager->provideOutputBufferForChannel(buffer, mOutputChannel);
        }
        
        
        void InputNode::process()
        {
            auto& buffer = getOutputBuffer(audioOutput);
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = mNodeManager->getInputSample(mInputChannel, i);
        }

    }
        
}

