
#include "ChannelNode.h"

#include <audio/core/audionodemanager.h>

namespace nap
{
    namespace audio
    {

        ChannelNode::ChannelNode(NodeManager& nodeManager) : Node(nodeManager), mZeroBuffer(nodeManager.getInternalBufferSize(), 0.)
        {

        }


        void ChannelNode::setOutputZero(bool outputZero)
        {
            mOutputZero = outputZero;
            
            if(mOutputZero)
            {
                // set audioOutput buffer to zero
                auto& outputBuffer = getOutputBuffer(audioOutput);
                outputBuffer = mZeroBuffer;
            }
        }
        
        
        SampleBuffer* ChannelNode::getLastInputBuffer()
        {
            return mLastInputBuffer;
        }
        
        
        void ChannelNode::setOutputBuffer(SampleBuffer* buffer)
        {
            if(!mOutputZero)
            {
                auto& outputBuffer = getOutputBuffer(audioOutput);
                outputBuffer = *buffer;
            }
        }
        

        void ChannelNode::process()
        {
            mLastInputBuffer = audioInput.pull();
        }

        
    }
}
