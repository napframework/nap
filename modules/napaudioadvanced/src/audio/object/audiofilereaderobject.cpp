#include "audiofilereaderobject.h"

RTTI_BEGIN_CLASS(nap::audio::AudioFileReaderObject)
    RTTI_PROPERTY("AudioFiles", &nap::audio::AudioFileReaderObject::mAudioFiles, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("BufferSize", &nap::audio::AudioFileReaderObject::mBufferSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileReaderObjectInstance)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> AudioFileReaderObject::createInstance(NodeManager &nodeManager, utility::ErrorState &errorState)
        {
            auto instance = std::make_unique<AudioFileReaderObjectInstance>();
            if (!instance->init(nodeManager, mAudioFiles, mBufferSize, errorState))
            {
                errorState.fail("Failed to initialize AudioFileReaderObjectInstance");
                return nullptr;
            }

            return std::move(instance);
        }


        bool AudioFileReaderObjectInstance::init(NodeManager &nodeManager, std::vector<ResourcePtr<AudioFileIO>>& audioFileReaders, int bufferSize, utility::ErrorState &errorState)
        {
            mAudioFiles = audioFileReaders;
            for (auto& audioFile : mAudioFiles)
            {
                if (audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::READ && audioFile->getDescriptor()->getMode() != AudioFileDescriptor::Mode::READWRITE)
                {
                    errorState.fail("AudioFileReaderObject: Audio file not opened for reading");
                    return false;
                }
                if (audioFile->getDescriptor()->getChannelCount() != 1)
                {
                    errorState.fail("AudioFileReaderObject works with mono AudioFileIO resources");
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

