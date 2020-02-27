#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/audiofilewriternode.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI AudioFileWriterObject : public AudioObject
        {
            RTTI_ENABLE(AudioObject)

        public:
            AudioFileWriterObject() = default;

            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles; ///< property: 'AudioFiles' Vector that points to mono @AudioFileWriter resources to write each channel of the object into.
            ResourcePtr<AudioObject> mInput = nullptr;

        private:
            std::unique_ptr<AudioObjectInstance> createInstance(NodeManager& nodeManager, utility::ErrorState& errorState) override;
        };


        class NAPAPI AudioFileWriterObjectInstance : public AudioObjectInstance
        {
            RTTI_ENABLE(AudioObjectInstance)

        public:
            AudioFileWriterObjectInstance() = default;
            AudioFileWriterObjectInstance(const std::string& name) : AudioObjectInstance(name) { }
            bool init(NodeManager& nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFiles, AudioObjectInstance* input, utility::ErrorState& errorState);

            int getChannelCount() const override { return 0; }
            OutputPin* getOutputForChannel(int channel) override { return nullptr; }
            int getInputChannelCount() const override { return mNodes.size(); }
            void connect(unsigned int channel, OutputPin& pin) override { mNodes[channel]->audioInput.connect(pin); }

        private:
            std::vector<ResourcePtr<AudioFileIO>> mAudioFiles;
            std::vector<SafeOwner<AudioFileWriterNode>> mNodes;
        };

    }

}