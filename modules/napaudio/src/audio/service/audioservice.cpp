// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>

// Audio includes
#include "audioservice.h"
#include <audio/resource/audiobufferresource.h>
#include <audio/resource/audiofileresource.h>

//#include <audio/core/graph.h>
//#include <audio/core/voice.h>

// Third party includes
#include <mpg123.h>


RTTI_BEGIN_CLASS(nap::audio::AudioServiceConfiguration)
	RTTI_PROPERTY("UseDefaultDevice",	&nap::audio::AudioServiceConfiguration::mUseDefaultDevice,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputDevice",		&nap::audio::AudioServiceConfiguration::mInputDevice,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputDevice",		&nap::audio::AudioServiceConfiguration::mOutputDevice,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputChannelCount",	&nap::audio::AudioServiceConfiguration::mInputChannelCount,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputChannelCount", &nap::audio::AudioServiceConfiguration::mOutputChannelCount,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleRate",			&nap::audio::AudioServiceConfiguration::mSampleRate,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize",			&nap::audio::AudioServiceConfiguration::mBufferSize,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
		/*
         * The audio callback will be called by portaudio to process a buffer of audio input/output
         */
        static int audioCallback( const void *inputBuffer, void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData )
        {
			float** out = (float**)outputBuffer;
			float** in = (float**)inputBuffer;

			AudioService* service = (AudioService*)userData;
			service->onAudioCallback(in, out, framesPerBuffer);

			return 0;
        }


        AudioService::AudioService(ServiceConfiguration* configuration) : 
			Service(configuration)
        {
            // Initialize mpg123 library
            mpg123_init();
        }

        
        AudioService::~AudioService()
        {
			Pa_StopStream(mStream);
			Pa_CloseStream(mStream);
			mStream = nullptr;
			Logger::info("Portaudio stopped");

            // Uninitialize mpg123 library
            mpg123_exit();
        }
        
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<AudioBufferResourceObjectCreator>(*this));
            factory.addObjectCreator(std::make_unique<AudioFileResourceObjectCreator>(*this));
        }

        
        NodeManager& AudioService::getNodeManager()
        {
            return mNodeManager;
        }
        
        
        bool AudioService::init(nap::utility::ErrorState& errorState)
        {
			AudioServiceConfiguration* configuration = getConfiguration<AudioServiceConfiguration>();

			PaError error = Pa_Initialize();
            if (error != paNoError)
            {
                std::string message = "Portaudio error: " + std::string(Pa_GetErrorText(error));
                errorState.fail(message);
                return false;
            }
            
            Logger::info("Portaudio initialized.");
            printDevices();
            
            // Initialize the audio device
			if (configuration->mInternalBufferSize % configuration->mBufferSize != 0)
			{
				errorState.fail("Internal buffer size does not fit device buffer size");
				return false;
			}

			if (configuration->mUseDefaultDevice)
				return startDefaultDevice(errorState);

			auto inputDeviceIndex = getDeviceIndex(configuration->mHostApi, configuration->mInputDevice);
			if (inputDeviceIndex < 0)
			{
				errorState.fail("Audio input device not found");
				return false;
			}

			auto outputDeviceIndex = getDeviceIndex(configuration->mHostApi, configuration->mOutputDevice);
			if (outputDeviceIndex < 0)
			{
				errorState.fail("Audio output device not found");
				return false;
			}

			PaStreamParameters inputParameters;
			inputParameters.device = inputDeviceIndex;
			inputParameters.channelCount = configuration->mInputChannelCount;
			inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
			inputParameters.suggestedLatency = 0;
			inputParameters.hostApiSpecificStreamInfo = nullptr;

			PaStreamParameters outputParameters;
			outputParameters.device = outputDeviceIndex;
			outputParameters.channelCount = configuration->mOutputChannelCount;
			outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
			outputParameters.suggestedLatency = 0;
			outputParameters.hostApiSpecificStreamInfo = nullptr;

			error = Pa_OpenStream(&mStream, &inputParameters, &outputParameters, configuration->mSampleRate, configuration->mBufferSize, paNoFlag, &audioCallback, this);
			if (error != paNoError)
			{
				errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
				return false;
			}

			mNodeManager.setInputChannelCount(configuration->mInputChannelCount);
			mNodeManager.setOutputChannelCount(configuration->mOutputChannelCount);
			mNodeManager.setSampleRate(configuration->mSampleRate);
			mNodeManager.setInternalBufferSize(configuration->mInternalBufferSize);

			error = Pa_StartStream(mStream);
			if (error != paNoError)
			{
				errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
				return false;
			}

			Logger::info("Portaudio stream started: %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", configuration->mInputDevice.c_str(), configuration->mOutputDevice.c_str(), configuration->mInputChannelCount, configuration->mOutputChannelCount, configuration->mSampleRate, configuration->mBufferSize);

			return true;
        }


		bool AudioService::startDefaultDevice(utility::ErrorState& errorState)
		{
			AudioServiceConfiguration* configuration = getConfiguration<AudioServiceConfiguration>();

			auto error = Pa_OpenDefaultStream(&mStream, configuration->mInputChannelCount, configuration->mOutputChannelCount, paFloat32 | paNonInterleaved, configuration->mSampleRate, configuration->mBufferSize, &audioCallback, this);
			if (error != paNoError)
			{
				errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
				return false;
			}

			mNodeManager.setInputChannelCount(configuration->mInputChannelCount);
			mNodeManager.setOutputChannelCount(configuration->mOutputChannelCount);
			mNodeManager.setSampleRate(configuration->mSampleRate);

			error = Pa_StartStream(mStream);
			if (error != paNoError)
			{
				errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
				return false;
			}

			auto hostApi = getHostApiName(Pa_GetDefaultHostApi());

			// Log the host API, devices, and channel, samplerate and buffersize being used
			if (configuration->mInputChannelCount <= 0)
			{
				// no input, only output
				auto outputDevice = getDeviceInfo(Pa_GetDefaultHostApi(), Pa_GetDefaultOutputDevice()).name;
				Logger::info("Portaudio default stream started: %s - %s, %i outputs, samplerate %i, buffersize %i", hostApi.c_str(), outputDevice, configuration->mOutputChannelCount, int(configuration->mSampleRate), configuration->mBufferSize);
			}
			else if (configuration->mOutputChannelCount <= 0)
			{
				// no output, input only
				auto inputDevice = getDeviceInfo(Pa_GetDefaultHostApi(), Pa_GetDefaultInputDevice()).name;
				Logger::info("Portaudio default stream started: %s - %s, %i inputs, samplerate %i, buffersize %i", hostApi.c_str(), inputDevice, configuration->mInputChannelCount, int(configuration->mSampleRate), configuration->mBufferSize);
			}
			else {
				auto inputDevice = getDeviceInfo(Pa_GetDefaultHostApi(), Pa_GetDefaultInputDevice()).name;
				auto outputDevice = getDeviceInfo(Pa_GetDefaultHostApi(), Pa_GetDefaultOutputDevice()).name;
				Logger::info("Portaudio default stream started: %s - %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", hostApi.c_str(), inputDevice, outputDevice, configuration->mInputChannelCount, configuration->mOutputChannelCount, int(configuration->mSampleRate), configuration->mBufferSize);
			}

			return true;
		}
        
        
        unsigned int AudioService::getHostApiCount()
        {
            return Pa_GetHostApiCount();
        }
        
        
        const PaHostApiInfo& AudioService::getHostApiInfo(unsigned int hostApiIndex)
        {
            return *Pa_GetHostApiInfo(hostApiIndex);
        }
        
        
        std::vector<const PaHostApiInfo*> AudioService::getHostApis()
        {
            std::vector<const PaHostApiInfo*> result;
            for (auto i = 0; i < Pa_GetHostApiCount(); ++i)
            {
                result.emplace_back(&getHostApiInfo(i));
            }
            return result;
        }
        
        
        std::string AudioService::getHostApiName(unsigned int hostApiIndex)
        {
            assert(hostApiIndex < getHostApiCount());
            return getHostApiInfo(hostApiIndex).name;
        }
        
        
        unsigned int AudioService::getDeviceCount(unsigned int hostApiIndex)
        {
            return getHostApiInfo(hostApiIndex).deviceCount;
        }
        
        
        const PaDeviceInfo& AudioService::getDeviceInfo(unsigned int hostApiIndex, unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex));
        }
        
        
        std::vector<const PaDeviceInfo*> AudioService::getDevices(unsigned int hostApiIndex)
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < getHostApiInfo(hostApiIndex).deviceCount; ++i)
            {
                result.emplace_back(&getDeviceInfo(hostApiIndex, i));
            }
            return result;
        }
        
        
        void AudioService::printDevices()
        {
            Logger::info("Available audio devices on this system:");
            for (auto hostApi = 0; hostApi < Pa_GetHostApiCount(); ++hostApi)
            {
                const PaHostApiInfo& hostApiInfo = *Pa_GetHostApiInfo(hostApi);
                nap::Logger::info("%s:", hostApiInfo.name);
                for (auto device = 0; device < hostApiInfo.deviceCount; ++device)
                {
                    auto index = Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device);
                    const PaDeviceInfo& info = *Pa_GetDeviceInfo(index);
                    nap::Logger::info("%i: %s %i inputs %i outputs", device, info.name, info.maxInputChannels, info.maxOutputChannels);
                }
            }
        }
        
        
        std::string AudioService::getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex)
        {
            assert(hostApiIndex < getHostApiCount());
            assert(deviceIndex < getDeviceCount(hostApiIndex));
            return getDeviceInfo(hostApiIndex, deviceIndex).name;
        }
        
        
        int AudioService::getDeviceIndex(const std::string& hostApi, const std::string& device)
        {
            for (auto hostApiIndex = 0; hostApiIndex < getHostApiCount(); ++hostApiIndex)
            {
                auto hostApiInfo = getHostApiInfo(hostApiIndex);
                if (nap::utility::toLower(hostApi) == nap::utility::toLower(hostApiInfo.name))
                {
                    for (auto deviceIndex = 0; deviceIndex < hostApiInfo.deviceCount; ++deviceIndex)
                    {
                        auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
                        if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name))
                            return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
                    }
                }
            }
            return -1;
        }


		void AudioService::shutdown()
		{
			auto error = Pa_Terminate();
			if (error != paNoError)
				Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
			Logger::info("Portaudio terminated");
		}
        
        
        void AudioService::onAudioCallback(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
        {
            // process the node manager
            mNodeManager.process(inputBuffer, outputBuffer, framesPerBuffer);
            
            // clean the trash bin with nodes and resources that are no longer used and scheduled for destruction
            mDeletionQueue.clear();
        }

    }
}
