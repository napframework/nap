#pragma once

// Nap includes
#include <nap/resource.h>
#include <rtti/factory.h>

// Audio includes
#include <audio/core/audionodemanager.h>

// Forward declarations
struct SNDFILE_tag;

namespace nap
{

    namespace audio
    {

        // Class used to wrap the SNDFILE* descriptor in a SafeOwner.
        class NAPAPI AudioFileDescriptor
        {
        public:
            enum class Mode { READ, WRITE, READWRITE };

        public:
            AudioFileDescriptor(const std::string& path, Mode mode, int channelCount = 1, float sampleRate = 44100.f);
            ~AudioFileDescriptor();
            bool isValid() { return mSndFile != nullptr; }

            /**
             * Writes multichannel interleaved data to the file.
             * @param buffer A vector containing multichannel interleaved audio sample data. The size of the buffer is required to be a multiple of the number of channels in the file.
             */
            unsigned int write(float* buffer, int size);

            /**
             * Reads multichannel interleaved data from the file.
             * @param buffer A vector containing multichannel interleaved audio sample data. The size of the buffer is required to be a multiple of the number of channels in the file.
             */
            unsigned int read(float* buffer, int size);

            /**
             * Moves the read/write position to the given offset.
             */
            void seek(DiscreteTimeValue offset);

            int getChannelCount() const { return mChannelCount; }
            float getSampleRate() const { return mSampleRate; }
            Mode getMode() const { return mMode; }

        private:
            SNDFILE_tag* mSndFile = nullptr;
            int mChannelCount = 1;
            float mSampleRate = 44100.f;
            Mode mMode = Mode::WRITE;
        };


        class NAPAPI AudioFileIO : public Resource {
            RTTI_ENABLE(Resource)

        public:
            AudioFileIO(NodeManager& nodeManager) : Resource(), mNodeManager(nodeManager) { }
            bool init(utility::ErrorState& errorState) override;

            std::string mPath = "";
            AudioFileDescriptor::Mode mMode = AudioFileDescriptor::Mode::WRITE;
            int mChannelCount = 1;

            SafePtr<AudioFileDescriptor> getDescriptor() { return mAudioFileDescriptor; }

        private:
            NodeManager& mNodeManager;
            SafeOwner<AudioFileDescriptor> mAudioFileDescriptor = nullptr;
        };


        using AudioFileIOObjectCreator = rtti::ObjectCreator<AudioFileIO, NodeManager>;


    }

}