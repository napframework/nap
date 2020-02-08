#include "audiofileio.h"

// Third party includes
#include <sndfile.h>

// Audio includes
#include <audio/core/audionodemanager.h>

RTTI_BEGIN_ENUM(nap::audio::AudioFileDescriptor::Mode)
RTTI_ENUM_VALUE(nap::audio::AudioFileDescriptor::Mode::WRITE, "Write"),
RTTI_ENUM_VALUE(nap::audio::AudioFileDescriptor::Mode::READ, "Read"),
RTTI_ENUM_VALUE(nap::audio::AudioFileDescriptor::Mode::READWRITE, "ReadWrite")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioFileIO)
    RTTI_CONSTRUCTOR(nap::audio::NodeManager&)
    RTTI_PROPERTY("Path", &nap::audio::AudioFileIO::mPath, nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("Mode", &nap::audio::AudioFileIO::mMode, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount", &nap::audio::AudioFileIO::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        AudioFileDescriptor::AudioFileDescriptor(const std::string& path, Mode mode, int channelCount, float sampleRate)
        {
            mMode = mode;
            SF_INFO sfInfo;
            sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
            if (mode == Mode::WRITE)
            {
                sfInfo.channels = channelCount;
                sfInfo.samplerate = sampleRate;
            }
            int libSndFileMode;
            if (mode == Mode::WRITE)
                libSndFileMode = SFM_WRITE;
            else if (mode == Mode::READ)
                libSndFileMode = SFM_READ;
            else
                libSndFileMode = SFM_RDWR;
            mSndFile = sf_open(path.c_str(), libSndFileMode, &sfInfo);
            mSampleRate = sfInfo.samplerate;
            mChannelCount = sfInfo.channels;
        }


        AudioFileDescriptor::~AudioFileDescriptor()
        {
            sf_close(mSndFile);
        }



        unsigned int AudioFileDescriptor::write(float* buffer, int size)
        {
            return sf_write_float(mSndFile, buffer, size);
        }


        unsigned int AudioFileDescriptor::read(float* destination, int size)
        {
            return sf_read_float(mSndFile, destination, size);
        }


        void AudioFileDescriptor::seek(DiscreteTimeValue offset)
        {
            sf_seek(mSndFile, offset, SEEK_SET);
        }



        bool AudioFileIO::init(utility::ErrorState& errorState)
        {
            mAudioFileDescriptor = mNodeManager.makeSafe<AudioFileDescriptor>(mPath, mMode, mChannelCount, mNodeManager.getSampleRate());

            if (!mAudioFileDescriptor->isValid())
            {
                errorState.fail("Failed to open audio file %s for writing", mPath.c_str());
                return false;
            }

            return true;
        }


    }
}