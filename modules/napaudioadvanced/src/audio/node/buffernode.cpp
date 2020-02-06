#include "buffernode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        
        BufferNode::BufferNode(NodeManager& nodeManager) : Node(nodeManager)
        {
            bufferSizeChanged(getBufferSize());
        }

        
        void BufferNode::update()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto inputBuffer = audioInput.pull();
            if (inputBuffer != nullptr)
                memcpy(outputBuffer.data(), inputBuffer->data(), mCacheSize);
            else
                memset(outputBuffer.data(), 0, mCacheSize);
        }
        
        
        void BufferNode::bufferSizeChanged(int size)
        {
            mCacheSize = size * sizeof(float);
            auto& outputBuffer = getOutputBuffer(audioOutput);
            memset(outputBuffer.data(), 0, mCacheSize);
        }
        
        
        void BufferUpdateProcess::process()
        {
            for (auto& buffer : mBuffers)
                buffer->update();
        }
        
        
        void BufferUpdateProcess::registerBuffer(BufferNode* buffer)
        {
            getNodeManager().enqueueTask([&, buffer](){
                auto it = std::find(mBuffers.begin(), mBuffers.end(), buffer);
                if (it == mBuffers.end())
                    mBuffers.emplace_back(buffer);
            });
        }
        
        
        void BufferUpdateProcess::unregisterBuffer(BufferNode* buffer)
        {
            getNodeManager().enqueueTask([&, buffer](){
                auto it = std::find(mBuffers.begin(), mBuffers.end(), buffer);
                if (it != mBuffers.end())
                    mBuffers.erase(it);
            });
        }
        


        
    }
    
}
