#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/audiofilereadernode.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI AudioFileReader : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            AudioFileReader() = default;

            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles; ///< property: 'AudioFiles' Vector that points to mono @AudioFileIO resources to read each channel of the object from.
            int mBufferSize = 65536;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        class NAPAPI AudioFileReaderInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            AudioFileReaderInstance() = default;
            AudioFileReaderInstance(const std::string& name) : AudioObjectInstance(name) { }
            bool init(NodeManager& nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFiles, int bufefrSize, utility::ErrorState& errorState);

            int getChannelCount() const override { return mNodes.size(); }
            OutputPin* getOutputForChannel(int channel) override { return &mNodes[channel]->audioOutput; }

        private:
            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles;
            std::vector<SafeOwner<AudioFileReaderNode>> mNodes;
        };

    }

}