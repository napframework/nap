#pragma once

// Nap includes
#include <utility/threading.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/resource/audiofileio.h>
#include <audio/utility/audiofunctions.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI AudioFileReaderNode : public Node
        {
            RTTI_ENABLE(Node)

        public:
            AudioFileReaderNode(NodeManager& nodeManager, unsigned int bufferSize);
            void setAudioFile(SafePtr<AudioFileDescriptor> audioFileDescriptor);
            bool isPlaying() const { return mAudioFileDescriptor != nullptr; }
            void setLooping(bool value) { mLooping = value; }
            bool isLooping() const { return mLooping; }

            OutputPin audioOutput = { this };

        private:
            void process() override;

            WorkerThread mThread;
            SafePtr<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
            SampleBuffer mCircularBuffer;
            SampleBuffer mDiskReadBuffer;
            DiscreteTimeValue mWritePosition = 0;
            double mReadPosition = 0;
            bool mLooping = false;
        };


    }

}