#include "circularbuffernode.h"

// Nap includes
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::CircularBufferNode)
    RTTI_PROPERTY("audioInput", &nap::audio::CircularBufferNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        
        CircularBufferNode::CircularBufferNode(NodeManager& nodeManager, unsigned int bufferSize, bool rootProcess) : Node(nodeManager), mRootProcess(rootProcess)
        {
//            TODO check if bufferSize is power of two
            mBuffer.resize(bufferSize);
            
            if (rootProcess)
                getNodeManager().registerRootProcess(*this);
        }
        
        
        CircularBufferNode::~CircularBufferNode()
        {
            if (mRootProcess)
                getNodeManager().unregisterRootProcess(*this);
        }
        
        
        void CircularBufferNode::process()
        {
            auto inputBuffer = audioInput.pull();
            if (inputBuffer == nullptr)
            {
                for (auto i = 0; i < getBufferSize(); ++i)
                {
                    mBuffer[wrap(mWritePosition, mBuffer.size())] = 0;
                    mWritePosition++;
                }
            }
            else {
                for (float i : *inputBuffer)
                {
                    mBuffer[wrap(mWritePosition, mBuffer.size())] = i;
                    mWritePosition++;
                }
            }
        }


    }
    
}
