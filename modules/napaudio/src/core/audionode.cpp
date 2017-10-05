#include "audionode.h"
#include "audionodemanager.h"

RTTI_DEFINE_BASE(nap::audio::Node)

namespace nap {
    
    namespace audio {
                        
        
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
        
        
        SampleBuffer& Node::getOutputBuffer(OutputPin& output) { return output.mBuffer; }

        
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
            getNodeManager().unregisterRootNode(*this);
        }
        
        
        void OutputNode::process()
        {
            if (!mActive)
                return;
            
            SampleBufferPtr buffer = audioInput.pull();
            if (buffer)
                getNodeManager().provideOutputBufferForChannel(buffer, mOutputChannel);
        }
        
        
        void InputNode::process()
        {
            auto& buffer = getOutputBuffer(audioOutput);
            for (auto i = 0; i < buffer.size(); ++i)
                buffer[i] = getNodeManager().getInputSample(mInputChannel, i);
        }

    }
        
}

