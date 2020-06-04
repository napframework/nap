#include "audiofilereader.h"

RTTI_BEGIN_CLASS(nap::audio::AudioFileReader)
    RTTI_PROPERTY("AudioFiles", &nap::audio::AudioFileReader::mAudioFiles, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("BufferSize", &nap::audio::AudioFileReader::mBufferSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileReaderInstance)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> AudioFileReader::createInstance(NodeManager &nodeManager, utility::ErrorState &errorState)
        {
            auto instance = std::make_unique<AudioFileReaderInstance>();
            if (!instance->init(nodeManager, mAudioFiles, mBufferSize, errorState))
            {
                errorState.fail("Failed to initialize AudioFileReaderInstance");
                return nullptr;
            }

            return std::move(instance);
        }


        bool AudioFileReaderInstance::init(NodeManager &nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFileReaders, int bufferSize, utility::ErrorState &errorState)
        {
            mAudioFiles = audioFileReaders;
            for (auto& audioFile : mAudioFiles)
            {
                if (audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::READ && audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::READWRITE)
                {
                    errorState.fail("AudioFileReader: Audio file not opened for reading");
                    return false;
                }
                if (audioFile->getDescriptor()->getChannelCount() != 1)
                {
                    errorState.fail("AudioFileReader works with mono AudioFileIO resources");
                    return false;
                }

                auto node = nodeManager.makeSafe<AudioFileReaderNode>(nodeManager, bufferSize);
                node->setAudioFile(audioFile->getDescriptor());
                mNodes.emplace_back(std::move(node));
            }

            return true;
        }
    }

}

