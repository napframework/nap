#include "audiofilewriter.h"

RTTI_BEGIN_CLASS(nap::audio::AudioFileWriter)
    RTTI_PROPERTY("AudioFiles", &nap::audio::AudioFileWriter::mAudioFiles, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Input", &nap::audio::AudioFileWriter::mInput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileWriterInstance)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> AudioFileWriter::createInstance(NodeManager &nodeManager, utility::ErrorState &errorState)
        {
            auto instance = std::make_unique<AudioFileWriterInstance>();
            if (!instance->init(nodeManager, mAudioFiles, mInput->getInstance(), errorState))
            {
                errorState.fail("Failed to initialize AudioFileWriterInstance");
                return nullptr;
            }

            return std::move(instance);
        }


        bool AudioFileWriterInstance::init(NodeManager &nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFileWriters, AudioObjectInstance* input, utility::ErrorState &errorState)
        {
            if (input != nullptr)
                if (input->getChannelCount() < 1)
                {
                    errorState.fail("AudioFileWriterInstance input needs to have at least 1 output channel");
                    return false;
                }

            mAudioFiles = audioFileWriters;
            int inputChannel = 0;
            for (auto& audioFile : mAudioFiles)
            {
                if (audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::WRITE && audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::READWRITE)
                {
                    errorState.fail("AudioFileWriter: Audio file not opened for writing");
                    return false;
                }
                if (audioFile->getDescriptor()->getChannelCount() != 1)
                {
                    errorState.fail("AudioFileWriter works with mono AudioFileWriter resources");
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

