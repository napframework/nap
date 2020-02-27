#include "audiofilewriternode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{

    namespace audio
    {


        AudioFileWriterNode::AudioFileWriterNode(NodeManager& nodeManager, int bufferQueueSize, bool rootProcess) : Node(nodeManager), mRootProcess(rootProcess)
        {
            mBufferQueue.resize(bufferQueueSize);
            for (auto& buffer : mBufferQueue)
                buffer.resize(nodeManager.getInternalBufferSize());
            mBufferSizeInBytes = sizeof(float) * getBufferSize();
            mThread.start();
            if (mRootProcess)
                nodeManager.registerRootProcess(*this);
        }


        AudioFileWriterNode::~AudioFileWriterNode()
        {
            if (mRootProcess)
                getNodeManager().unregisterRootProcess(*this);
        }


        void AudioFileWriterNode::bufferSizeChanged(int size)
        {
            getNodeManager().enqueueTask([&](){
                mThread.stop();
                for (auto& buffer : mBufferQueue)
                    buffer.resize(size);
                mBufferSizeInBytes = sizeof(float) * getBufferSize();
                mInputIndex = 0;
                mDiskWriteIndex = 0;
                mThread.start();
            });
        }


        void AudioFileWriterNode::process()
        {
            auto inputBuffer = audioInput.pull();
            std::memcpy(mBufferQueue[mInputIndex].data(), inputBuffer->data(), mBufferSizeInBytes);
            mThread.enqueue([&](){
                if (mAudioFileDescriptor != nullptr)
                    mAudioFileDescriptor->write(mBufferQueue[mDiskWriteIndex].data(), getBufferSize());
                mDiskWriteIndex++;
                if (mDiskWriteIndex >= mBufferQueue.size())
                    mDiskWriteIndex = 0;
            });
            mInputIndex++;
            if (mInputIndex >= mBufferQueue.size())
                mInputIndex = 0;
        }

    }

}
