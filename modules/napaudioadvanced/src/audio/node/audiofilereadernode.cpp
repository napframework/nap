#include "audiofilereadernode.h"

// Audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{

    namespace audio
    {


        AudioFileReaderNode::AudioFileReaderNode(NodeManager& nodeManager, unsigned int bufferSize) : Node(nodeManager)
        {
            mCircularBuffer.resize(bufferSize);
            mDiskReadBuffer.resize(getBufferSize());

            mThread.start();
        }


        void AudioFileReaderNode::setAudioFile(SafePtr<AudioFileDescriptor> audioFileDescriptor)
        {
            mAudioFileDescriptor = audioFileDescriptor;
            mWritePosition = 0;
            mReadPosition = 0;
            if (mAudioFileDescriptor != nullptr)
            {
                auto framesRead = mAudioFileDescriptor->read(&mCircularBuffer[0], mDiskReadBuffer.size());
                if (framesRead != mDiskReadBuffer.size())
                {
                    if (mLooping)
                        mAudioFileDescriptor->seek(0);
                    else
                        mAudioFileDescriptor = nullptr;
                }
                mWritePosition = framesRead;
            }
        }


        void AudioFileReaderNode::process()
        {
            auto& outputBuffer = getOutputBuffer(audioOutput);

            if (mAudioFileDescriptor == nullptr)
            {
                for (auto i = 0; i < outputBuffer.size(); ++i)
                    outputBuffer[i] = 0.f;
                return;
            }

            for (auto i = 0; i < outputBuffer.size(); ++i)
            {
                if (mReadPosition < mWritePosition)
                {
                    int mReadPositionFloor = int(mReadPosition);
                    float remainder = mReadPosition - mReadPositionFloor;
                    SampleValue start = mCircularBuffer[wrap(mReadPositionFloor, mCircularBuffer.size())];
                    SampleValue end = mCircularBuffer[wrap(mReadPositionFloor + 1, mCircularBuffer.size())];
                    auto value = lerp(start, end, remainder);
                    outputBuffer[i] = value;
                    mReadPosition += mAudioFileDescriptor->getSampleRate() / getNodeManager().getSampleRate();
                }
                else
                    outputBuffer[i] = 0.f;
            }
            if (mReadPosition > mWritePosition - mDiskReadBuffer.size())
            {
               mThread.enqueue([&](){
                   if (mAudioFileDescriptor == nullptr)
                       return;
                   auto framesRead = mAudioFileDescriptor->read(mDiskReadBuffer.data(), mDiskReadBuffer.size());
                   if (framesRead != mDiskReadBuffer.size())
                   {
                       if (mLooping)
                           mAudioFileDescriptor->seek(0);
                       else
                           mAudioFileDescriptor = nullptr;
                   }
                   int wrappedWritePos = wrap(mWritePosition, mCircularBuffer.size());
                   for (auto i = 0; i < framesRead; ++i)
                   {
                       mCircularBuffer[wrappedWritePos] = mDiskReadBuffer[i];
                       wrappedWritePos++;
                       if (wrappedWritePos >= mCircularBuffer.size())
                           wrappedWritePos = 0;
                   }
                   mWritePosition += framesRead;
               });
            }
        }

    }

}
