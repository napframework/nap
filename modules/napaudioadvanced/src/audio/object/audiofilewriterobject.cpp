#include "audiofilewriterobject.h"

RTTI_BEGIN_CLASS(nap::audio::AudioFileWriterObject)
    RTTI_PROPERTY("AudioFiles", &nap::audio::AudioFileWriterObject::mAudioFiles, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Input", &nap::audio::AudioFileWriterObject::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileWriterObjectInstance)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> AudioFileWriterObject::createInstance(NodeManager &nodeManager, utility::ErrorState &errorState)
        {
            auto instance = std::make_unique<AudioFileWriterObjectInstance>();
            if (!instance->init(nodeManager, mAudioFiles, mInput->getInstance(), errorState))
            {
                errorState.fail("Failed to initialize AudioFileWriterObjectInstance");
                return nullptr;
            }

            return std::move(instance);
        }


        bool AudioFileWriterObjectInstance::init(NodeManager &nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFileWriters, AudioObjectInstance* input, utility::ErrorState &errorState)
        {
            if (input != nullptr)
                if (input->getChannelCount() < 1)
                {
                    errorState.fail("AudioFileWriterObjectInstance input needs to have at least 1 output channel");
                    return false;
                }

            mAudioFiles = audioFileWriters;
            int inputChannel = 0;
            for (auto& audioFile : mAudioFiles)
            {
                if (audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::WRITE && audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::READWRITE)
                {
                    errorState.fail("AudioFileWriterObject: Audio file not opened for writing");
                    return false;
                }
                if (audioFile->getDescriptor()->getChannelCount() != 1)
                {
                    errorState.fail("AudioFileWriterObject works with mono AudioFileWriter resources");
                    return false;
                }

                auto node = nodeManager.makeSafe<AudioFileWriterNode>(nodeManager, 4, true);
                node->setAudioFile(audioFile->getDescriptor());
                if (input != nullptr)
                    node->audioInput.connect(*input->getOutputForChannel(inputChannel % input->getChannelCount()));
                inputChannel++;
                mNodes.emplace_back(std::move(node));
            }

            return true;
        }
    }

}

