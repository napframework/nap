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
    RTTI_PROPERTY("HostApi",            &nap::audio::AudioServiceConfiguration::mHostApi,            nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputDevice",		&nap::audio::AudioServiceConfiguration::mInputDevice,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputDevice",		&nap::audio::AudioServiceConfiguration::mOutputDevice,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputChannelCount",	&nap::audio::AudioServiceConfiguration::mInputChannelCount,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputChannelCount", &nap::audio::AudioServiceConfiguration::mOutputChannelCount,	nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AllowChannelCountFailure", &nap::audio::AudioServiceConfiguration::mAllowChannelCountFailure,    nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleRate",			&nap::audio::AudioServiceConfiguration::mSampleRate,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize",			&nap::audio::AudioServiceConfiguration::mBufferSize,			nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("InternalBufferSize", &nap::audio::AudioServiceConfiguration::mInternalBufferSize,            nap::rtti::EPropertyMetaData::Default)
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
            
            checkLockfreeTypes();
        }

        
        AudioService::~AudioService()
        {
			Pa_StopStream(mStream);
			Pa_CloseStream(mStream);
			mStream = nullptr;

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
			if (configuration->mBufferSize % configuration->mInternalBufferSize != 0)
			{
				errorState.fail("AudioService: Internal buffer size does not fit device buffer size");
				return false;
			}
            
            auto hostApiIndex = -1;
            if (configuration->mHostApi.empty())
                hostApiIndex = Pa_GetDefaultHostApi();
            else
                hostApiIndex = getHostApiIndex(configuration->mHostApi);
            if (hostApiIndex < 0)
            {
                errorState.fail("Audio host API not found: %s", configuration->mHostApi.c_str());
                return false;
            }

            auto inputDeviceIndex = -1;
            if (configuration->mInputDevice.empty())
                inputDeviceIndex = Pa_GetDefaultInputDevice();
            else
                inputDeviceIndex = getDeviceIndex(hostApiIndex, configuration->mInputDevice);
            
			// Ensure the input index is valid
			if (inputDeviceIndex < 0)
            {
                if (configuration->mInputChannelCount == 0 || configuration->mAllowChannelCountFailure)
                {
                    Logger::info("Audio input device not found: %s", configuration->mInputDevice.c_str());
                }
                else {
                    errorState.fail("Audio input device not found: %s", configuration->mInputDevice.c_str());
                    return false;
                }
            }

            auto outputDeviceIndex = -1;
            if (configuration->mOutputDevice.empty())
                outputDeviceIndex = Pa_GetDefaultOutputDevice();
            else
                outputDeviceIndex = getDeviceIndex(hostApiIndex, configuration->mOutputDevice);
            
			// Ensure the output index is valid
			if (outputDeviceIndex < 0)
            {
                if (configuration->mInputChannelCount == 0 || configuration->mAllowChannelCountFailure)
                {
                    Logger::info("Audio output device not found: %s", configuration->mOutputDevice.c_str());
                }
                else {
                    errorState.fail("Audio output device not found: %s", configuration->mOutputDevice.c_str());
                    return false;
                }
            }

			// Ensure requested number of input and output channels is available
            if (!checkChannelCounts(inputDeviceIndex, outputDeviceIndex, errorState))
                return false;
            
			PaStreamParameters inputParameters;
			inputParameters.device = inputDeviceIndex;
			inputParameters.channelCount = mInputChannelCount;
			inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
			inputParameters.suggestedLatency = 0;
			inputParameters.hostApiSpecificStreamInfo = nullptr;

			PaStreamParameters outputParameters;
			outputParameters.device = outputDeviceIndex;
			outputParameters.channelCount = mOutputChannelCount;
			outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
			outputParameters.suggestedLatency = 0;
			outputParameters.hostApiSpecificStreamInfo = nullptr;
            
			// Setup input parameters
            PaStreamParameters* inputParamsPtr = nullptr;
			if (mInputChannelCount > 0)
			{
				assert(inputDeviceIndex >= 0);
				inputParamsPtr = &inputParameters;
			}

			// Setup output parameters
            PaStreamParameters* outputParamsPtr = nullptr;
			if (mOutputChannelCount > 0)
			{
				assert(outputDeviceIndex >= 0);
				outputParamsPtr = &outputParameters;
			}
            
			// Open stream
			error = Pa_OpenStream(&mStream, inputParamsPtr, outputParamsPtr, configuration->mSampleRate, configuration->mBufferSize, paNoFlag, &audioCallback, this);
			if (error != paNoError)
			{
				errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
				return false;
			}

			mNodeManager.setInputChannelCount(mInputChannelCount);
			mNodeManager.setOutputChannelCount(mOutputChannelCount);
			mNodeManager.setSampleRate(configuration->mSampleRate);
			mNodeManager.setInternalBufferSize(configuration->mInternalBufferSize);

			error = Pa_StartStream(mStream);
			if (error != paNoError)
			{
				errorState.fail("Portaudio error: " + std::string(Pa_GetErrorText(error)));
				return false;
			}

			Logger::info("Portaudio stream started: %s, %s, %i inputs, %i outputs, samplerate %i, buffersize %i", configuration->mInputDevice.c_str(), configuration->mOutputDevice.c_str(), mInputChannelCount, mOutputChannelCount, int(configuration->mSampleRate), configuration->mBufferSize);

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
        
        
        int AudioService::getDeviceIndex(int hostApiIndex, const std::string& device)
        {
            for (auto deviceIndex = 0; deviceIndex < getHostApiInfo(hostApiIndex).deviceCount; ++deviceIndex)
            {
                auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
                if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name))
                    return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
            }
            
            return -1;
        }
        
        
        int AudioService::getHostApiIndex(const std::string& hostApi)
        {
            auto hostApiIndex = -1;
            
            for (auto i = 0; i < getHostApiCount(); ++i)
                if (nap::utility::toLower(hostApi) == nap::utility::toLower(getHostApiInfo(i).name))
                    hostApiIndex = i;
            
            return hostApiIndex;
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

        
        bool AudioService::checkChannelCounts(int inputDeviceIndex, int outputDeviceIndex, utility::ErrorState& errorState)
        {
            AudioServiceConfiguration* configuration = getConfiguration<AudioServiceConfiguration>();
            
            const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(inputDeviceIndex);
            const PaDeviceInfo* outputDeviceInfo = Pa_GetDeviceInfo(outputDeviceIndex);
            
            if (!inputDeviceInfo)
            {
                // There is no input device
                if (configuration->mInputChannelCount > 0)
                {
                    if (configuration->mAllowChannelCountFailure)
                    {
                        Logger::warn("AudioService: input device not found, initializing without input channels.");
                        mInputChannelCount = 0;
                    }
                    else {
                        errorState.fail("AudioService: input device not found.");
                        return false;
                    }
                }
                else
                    mInputChannelCount = 0;
            }
            else {
                // There is an input device
                if (configuration->mInputChannelCount > inputDeviceInfo->maxInputChannels)
                {
                    // There are less channels than requested
                    if (configuration->mAllowChannelCountFailure)
                    {
                        Logger::warn("AudioService: Requested number of %i input channels not available, initializing with only %i", configuration->mInputChannelCount, inputDeviceInfo->maxInputChannels);
                        mInputChannelCount = inputDeviceInfo->maxInputChannels;
                    }
                    else {
                        errorState.fail("AudioService: Not enough available input channels on chosen device.");
                        return false;
                    }
                }
                else
                    // There are enough channels
                    mInputChannelCount = configuration->mInputChannelCount;
            }
            
            
            if (!outputDeviceInfo)
            {
                // There is no input device
                if (configuration->mOutputChannelCount > 0)
                {
                    if (configuration->mAllowChannelCountFailure)
                    {
                        Logger::warn("AudioService: output device not found, initializing without output channels.");
                        mOutputChannelCount = 0;
                    }
                    else {
                        errorState.fail("AudioService: output device not found.");
                        return false;
                    }
                }
                else
                    mOutputChannelCount = 0;
            }
            else {
                // There is an output device
                if (configuration->mOutputChannelCount > outputDeviceInfo->maxOutputChannels)
                {
                    // There are less channels than requested
                    if (configuration->mAllowChannelCountFailure)
                    {
                        Logger::warn("AudioService: Requested number of %i output channels not available, initializing with only %i", configuration->mOutputChannelCount, outputDeviceInfo->maxOutputChannels);
                        mOutputChannelCount = outputDeviceInfo->maxOutputChannels;
                    }
                    else {
                        errorState.fail("AudioService: Not enough available output channels on chosen device.");
                        return false;
                    }
                }
                else
                    // There are enough channels
                    mOutputChannelCount = configuration->mOutputChannelCount;
            }
            
			// One input channel must be specified
			if (mOutputChannelCount == 0 && mInputChannelCount == 0)
			{
				errorState.fail("AudioService: No input or output channel selected");
				return false;
			}

            return true;
        }
        
        
        void AudioService::checkLockfreeTypes()
        {
            /**
             * Currently this is diabled because atomic<T>::is_lock_free is unavailable in gcc < 4.8
             */

//            enum EnumType { a, b, c };
//            std::atomic<bool> boolVar;
//            std::atomic<int> intVar;
//            std::atomic<float> floatVar;
//            std::atomic<double> doubleVar;
//            std::atomic<long> longVar;
//            std::atomic<long double> longDoubleVar;
//            std::atomic<EnumType> enumVar;
//
//            if (!boolVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<bool>");
//            if (!intVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<int>");
//            if (!floatVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<float>");
//            if (!doubleVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<double>");
//            if (!longVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<long>");
//            if (!longDoubleVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic<long double>");
//            if (!enumVar.is_lock_free())
//                Logger::warn("%s is not lockfree on current platform", "atomic enum");
        }

    }
    
}
