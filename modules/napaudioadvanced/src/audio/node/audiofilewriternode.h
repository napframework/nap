#pragma once

// Nap includes
#include <utility/threading.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/resource/audiofileio.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI AudioFileWriterNode : public Node
        {
            RTTI_ENABLE(Node)

        public:
            AudioFileWriterNode(NodeManager& nodeManager, int bufferQueueSize = 4, bool rootProcess = true);
            ~AudioFileWriterNode();
            void setAudioFile(SafePtr<AudioFileDescriptor> audioFileDescriptor)
            {
                mAudioFileDescriptor = audioFileDescriptor;
            }

            InputPin audioInput = { this };

        private:
            void process() override;
            void bufferSizeChanged(int) override;

            std::vector<SampleBuffer> mBufferQueue;
            int mInputIndex = 0;
            int mDiskWriteIndex = 0;
            WorkerThread mThread;
            int mBufferSizeInBytes = 0;
            SafePtr<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
            bool mRootProcess = false;
        };


    }

}