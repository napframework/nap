/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>

// Audio includes
#include "audioservice.h"
#include <audio/resource/audiobufferresource.h>
#include <audio/resource/audiofileresource.h>

// Third party includes
#include <mpg123.h>

RTTI_BEGIN_STRUCT(nap::audio::AudioServiceConfiguration::DeviceSettings)
        RTTI_PROPERTY("HostApi", &nap::audio::AudioServiceConfiguration::DeviceSettings::mHostApi,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("InputDevice", &nap::audio::AudioServiceConfiguration::DeviceSettings::mInputDevice,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("OutputDevice", &nap::audio::AudioServiceConfiguration::DeviceSettings::mOutputDevice,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("InputChannelCount", &nap::audio::AudioServiceConfiguration::DeviceSettings::mInputChannelCount,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("OutputChannelCount", &nap::audio::AudioServiceConfiguration::DeviceSettings::mOutputChannelCount,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("DisableInput", &nap::audio::AudioServiceConfiguration::DeviceSettings::mDisableInput,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("DisableOutput", &nap::audio::AudioServiceConfiguration::DeviceSettings::mDisableOutput,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("SampleRate", &nap::audio::AudioServiceConfiguration::DeviceSettings::mSampleRate,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("BufferSize", &nap::audio::AudioServiceConfiguration::DeviceSettings::mBufferSize,
                      nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("InternalBufferSize", &nap::audio::AudioServiceConfiguration::DeviceSettings::mInternalBufferSize,
                      nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::audio::AudioServiceConfiguration)
        RTTI_PROPERTY("Audio Device Settings", &nap::audio::AudioServiceConfiguration::mDeviceSettings, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("AllowChannelCountFailure", &nap::audio::AudioServiceConfiguration::mAllowChannelCountFailure, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("AllowDeviceFailure", &nap::audio::AudioServiceConfiguration::mAllowDeviceFailure, nap::rtti::EPropertyMetaData::Default)
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
		static int audioCallback(const void* inputBuffer, void* outputBuffer,
		                         unsigned long framesPerBuffer,
		                         const PaStreamCallbackTimeInfo* timeInfo,
		                         PaStreamCallbackFlags statusFlags,
		                         void* userData)
		{
			float** out = (float**) outputBuffer;
			float** in = (float**) inputBuffer;
			
			AudioService* service = (AudioService*) userData;
			service->onAudioCallback(in, out, framesPerBuffer);
			
			return 0;
		}
		
		
		AudioService::AudioService(ServiceConfiguration* configuration) :
				Service(configuration), mNodeManager(mDeletionQueue)
		{
		}
		
		
		void AudioService::registerObjectCreators(rtti::Factory& factory)
		{
			factory.addObjectCreator(std::make_unique<AudioBufferResourceObjectCreator>(*this));
			factory.addObjectCreator(std::make_unique<AudioFileResourceObjectCreator>(*this));
			factory.addObjectCreator(std::make_unique<MultiAudioFileResourceObjectCreator>(*this));
		}
		
		
		bool AudioService::init(nap::utility::ErrorState& errorState)
		{
			// Initialize mpg123 library
			mpg123_init();
			mMpg123Initialized = true;
			checkLockfreeTypes();

			auto* configuration = getConfiguration<AudioServiceConfiguration>();
            const auto& device_settings = configuration->mDeviceSettings;

			// Initialize the portaudio library
			PaError error = Pa_Initialize();
			if (!errorState.check(error == paNoError, "Portaudio error: %s", Pa_GetErrorText(error)))
				return false;

			mPortAudioInitialized = true;
			Logger::info("Portaudio initialized");
			printDevices();

			if (!(openStream(device_settings, errorState) && start(errorState)))
			{
				if (!configuration->mAllowDeviceFailure)
				{
					return false;
				}
				else
                {
					Logger::warn(
                            "Portaudio stream failed to start with: input: %s (%i channels), output: %s (%i channels), samplerate %i, buffersize %i",
                            !device_settings.mInputDevice.empty() ? device_settings.mInputDevice.c_str() : "no input device", device_settings.mInputChannelCount,
                            !device_settings.mOutputDevice.empty() ? device_settings.mOutputDevice.c_str() : "no output device", device_settings.mOutputChannelCount, int(device_settings.mSampleRate),
                            device_settings.mBufferSize);
				}
				return true;
			}

			// Log portaudio stream settings
			Logger::info("Portaudio stream started:");
			if (mInputDeviceIndex >= 0)
				Logger::info("Input device: %s, %i channel(s)", Pa_GetDeviceInfo(mInputDeviceIndex)->name, mNodeManager.getInputChannelCount());
			else
				Logger::info("No input device");

			if (mOutputDeviceIndex >= 0)
				Logger::info("Output device: %s, %i channel(s)", Pa_GetDeviceInfo(mOutputDeviceIndex)->name, mNodeManager.getOutputChannelCount());
			else
				Logger::info("No output device");
			Logger::info("Samplerate: %i", int(mNodeManager.getSampleRate()));
			Logger::info("Buffersize: %i", device_settings.mBufferSize);

			return true;
		}


        bool AudioService::openStream(const AudioServiceConfiguration::DeviceSettings& deviceSettings, utility::ErrorState& errorState)
        {
            auto result = _openStream(deviceSettings, errorState);
            mErrorMessage = errorState.toString();
            return result;
        }


        bool AudioService::_openStream(const AudioServiceConfiguration::DeviceSettings& deviceSettings, utility::ErrorState& errorState)
        {
            // copy settings to configuration
            auto* configuration = getConfiguration<AudioServiceConfiguration>();
            configuration->mDeviceSettings = deviceSettings;

            // close stream
            if(isOpened())
            {
                if(!closeStream(errorState))
                    return false;
            }

            int inputDeviceIndex = -1;
            int outputDeviceIndex = -1;
            int inputChannelCount = 0;
            int outputChannelCount = 0;

            // Initialize the audio device
            if (deviceSettings.mBufferSize % deviceSettings.mInternalBufferSize != 0) {
                errorState.fail("AudioService: Internal buffer size does not fit device buffer size");
                return false;
            }

            if (deviceSettings.mHostApi.empty())
                mHostApiIndex = Pa_GetDefaultHostApi();
            else
                mHostApiIndex = getHostApiIndex(deviceSettings.mHostApi);
            if (mHostApiIndex < 0)
            {
                errorState.fail("Audio host API not found: %s", deviceSettings.mHostApi.c_str());
                return false;
            }

            if (deviceSettings.mDisableInput)
            {
                inputDeviceIndex = -1;
            }
            else {
                if (deviceSettings.mInputDevice.empty())
                    inputDeviceIndex = Pa_GetDefaultInputDevice();
                else
                    inputDeviceIndex = getInputDeviceIndex(mHostApiIndex, deviceSettings.mInputDevice);
                if (inputDeviceIndex < 0)
                {
                    errorState.fail("Audio input device not found: %s", deviceSettings.mInputDevice.c_str());
                    return false;
                }
            }

            if (deviceSettings.mDisableOutput)
            {
                outputDeviceIndex = -1;
            }
            else {
                if (deviceSettings.mOutputDevice.empty())
                    outputDeviceIndex = Pa_GetDefaultOutputDevice();
                else
                    outputDeviceIndex = getOutputDeviceIndex(mHostApiIndex, deviceSettings.mOutputDevice);
                if (outputDeviceIndex < 0)
                {
                    errorState.fail("Audio output device not found: %s", deviceSettings.mOutputDevice.c_str());
                    return false;
                }
            }

            if (inputDeviceIndex < 0 && outputDeviceIndex < 0)
            {
                errorState.fail("Cannot start audio stream with neither input nor output.");
                return false;
            }

            if (!checkChannelCounts(inputDeviceIndex, outputDeviceIndex, inputChannelCount, outputChannelCount, errorState))
                return false;

            mInputDeviceIndex = inputDeviceIndex;
            mOutputDeviceIndex = outputDeviceIndex;

            return openStream(errorState);
        }


        const AudioServiceConfiguration::DeviceSettings& AudioService::getDeviceSettings() const
        {
            auto* configuration = getConfiguration<AudioServiceConfiguration>();
            return configuration->mDeviceSettings;
        }


		void AudioService::shutdown()
		{
			// First close port-audio, only do so when initialized
			if (mPortAudioInitialized)
			{
				Pa_StopStream(mStream);

				// Close stream
				Pa_CloseStream(mStream);
				mStream = nullptr;

				// Terminate Portaudio
				auto error = Pa_Terminate();
				if (error != paNoError) 
					Logger::warn("Portaudio error: %s", Pa_GetErrorText(error));
			}

			// Close mpg123 library
			if(mMpg123Initialized)
				mpg123_exit();
		}


		NodeManager& AudioService::getNodeManager()
		{
			return mNodeManager;
		}


        bool AudioService::openStream(utility::ErrorState& errorState)
        {
            // The stream can only be opened when it's closed
            assert(mStream == nullptr);

            const auto& device_settings = getConfiguration<AudioServiceConfiguration>()->mDeviceSettings;

            mNodeManager.setInputChannelCount(device_settings.mDisableInput ? 0 : device_settings.mInputChannelCount);
            mNodeManager.setOutputChannelCount(device_settings.mDisableOutput ? 0 : device_settings.mOutputChannelCount);
            mNodeManager.setSampleRate(device_settings.mSampleRate);
            mNodeManager.setInternalBufferSize(device_settings.mInternalBufferSize);

            PaStreamParameters inputParameters;
            inputParameters.device = mInputDeviceIndex;
            inputParameters.channelCount = mNodeManager.getInputChannelCount();
            inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            inputParameters.hostApiSpecificStreamInfo = nullptr;

            PaStreamParameters outputParameters;
            outputParameters.device = mOutputDeviceIndex;
            outputParameters.channelCount = mNodeManager.getOutputChannelCount();
            outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            outputParameters.hostApiSpecificStreamInfo = nullptr;

            PaStreamParameters* inputParamsPtr = nullptr;
            if (mInputDeviceIndex >= 0)
            {
                inputParameters.suggestedLatency = Pa_GetDeviceInfo(mInputDeviceIndex)->defaultLowInputLatency;
                inputParamsPtr = &inputParameters;
            }
            PaStreamParameters* outputParamsPtr = nullptr;
            if (mOutputDeviceIndex >= 0)
            {
                outputParameters.suggestedLatency = Pa_GetDeviceInfo(mOutputDeviceIndex)->defaultLowOutputLatency;
                outputParamsPtr = &outputParameters;
            }

            PaError error = Pa_OpenStream(&mStream, inputParamsPtr, outputParamsPtr, mNodeManager.getSampleRate(), device_settings.mBufferSize, paNoFlag, &audioCallback, this);
            if (error != paNoError)
            {
                errorState.fail("Error opening audio stream: %s", Pa_GetErrorText(error));
                mInputDeviceIndex = -1;
                mOutputDeviceIndex = -1;
                mStream = nullptr;

                return false;
            }

            return true;
        }
		
		
		bool AudioService::closeStream(utility::ErrorState& errorState)
		{
			assert(mStream != nullptr);
			
			auto paError = Pa_CloseStream(mStream);
			if (paError != paNoError)
			{
				errorState.fail("Failed to close audio stream: %s", Pa_GetErrorText(paError));
				return false;
			}
			mStream = nullptr;
			
			return true;
		}
		
		
		bool AudioService::stop(utility::ErrorState& errorState)
		{
			assert(mStream != nullptr);
			
			auto paError = Pa_StopStream(mStream);
			if (paError != paNoError)
			{
				errorState.fail("Failed to pause audio stream: %s", Pa_GetErrorText(paError));
				return false;
			}
			
			return true;
		}
		
		
		bool AudioService::start(utility::ErrorState& errorState)
		{
			assert(mStream != nullptr);
			
			auto paError = Pa_StartStream(mStream);
			if (paError != paNoError)
			{
				errorState.fail("Failed to start audio stream: %s", Pa_GetErrorText(paError));
				return false;
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
					nap::Logger::info("%i: %s, %i input(s), %i output(s)", device, info.name, info.maxInputChannels,
					                  info.maxOutputChannels);
				}
			}
		}
		
		
		std::string AudioService::getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex)
		{
			assert(hostApiIndex < getHostApiCount());
			assert(deviceIndex < getDeviceCount(hostApiIndex));
			return getDeviceInfo(hostApiIndex, deviceIndex).name;
		}
		
		
		int AudioService::getInputDeviceIndex(int hostApiIndex, const std::string& device)
		{
			for (auto deviceIndex = 0; deviceIndex < getHostApiInfo(hostApiIndex).deviceCount; ++deviceIndex)
			{
				auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
				if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name) && deviceInfo.maxInputChannels > 0)
					return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
			}
			
			return -1;
		}


        int AudioService::getOutputDeviceIndex(int hostApiIndex, const std::string& device)
        {
            for (auto deviceIndex = 0; deviceIndex < getHostApiInfo(hostApiIndex).deviceCount; ++deviceIndex)
            {
                auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
                if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name) && deviceInfo.maxOutputChannels > 0)
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
		
		
		void AudioService::onAudioCallback(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
		{
			// process the node manager
			mNodeManager.process(inputBuffer, outputBuffer, framesPerBuffer);
			
			// clean the trash bin with nodes and resources that are no longer used and scheduled for destruction
			mDeletionQueue.clear();
		}
		
		
		bool AudioService::checkChannelCounts(int inputDeviceIndex, int outputDeviceIndex, int& inputChannelCount,
		                                      int& outputChannelCount, utility::ErrorState& errorState)
		{
			AudioServiceConfiguration* configuration = getConfiguration<AudioServiceConfiguration>();
            const auto& settings = configuration->mDeviceSettings;
			
			const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(inputDeviceIndex);
			const PaDeviceInfo* outputDeviceInfo = Pa_GetDeviceInfo(outputDeviceIndex);
			
			if (inputDeviceInfo == nullptr)
			{
				// There is no input device
				if (settings.mDisableInput == false && settings.mInputChannelCount > 0)
				{
					// Input channels were requested
					if (configuration->mAllowChannelCountFailure)
					{
						Logger::warn("AudioService: input device not found, initializing without input channels.");
						inputChannelCount = 0;
					}
					else {
						errorState.fail("AudioService: input device not found.");
						return false;
					}
				}
				else
					inputChannelCount = 0;
			}
			else {
				// There is an input device
				if (settings.mInputChannelCount > inputDeviceInfo->maxInputChannels)
				{
					// There are less channels than requested
					if (configuration->mAllowChannelCountFailure)
					{
						Logger::warn(
								"AudioService: Requested number of %i input channels not available, initializing with only %i",
                                settings.mInputChannelCount, inputDeviceInfo->maxInputChannels);
						inputChannelCount = inputDeviceInfo->maxInputChannels;
					}
					else {
						errorState.fail("AudioService: Not enough available input channels on chosen device.");
						return false;
					}
				}
				else
					// There are enough channels
					inputChannelCount = settings.mInputChannelCount;
			}
			
			
			if (!outputDeviceInfo)
			{
				// There is no output device
				if (settings.mDisableOutput == false && settings.mOutputChannelCount > 0)
				{
					if (configuration->mAllowChannelCountFailure)
					{
						Logger::warn("AudioService: output device not found, initializing without output channels.");
						outputChannelCount = 0;
					}
					else {
						errorState.fail("AudioService: output device not found.");
						return false;
					}
				}
				else
					outputChannelCount = 0;
			}
			else {
				// There is an output device
				if (settings.mOutputChannelCount > outputDeviceInfo->maxOutputChannels)
				{
					// There are less channels than requested
					if (configuration->mAllowChannelCountFailure)
					{
						Logger::warn(
								"AudioService: Requested number of %i output channels not available, initializing with only %i",
                                settings.mOutputChannelCount, outputDeviceInfo->maxOutputChannels);
						outputChannelCount = outputDeviceInfo->maxOutputChannels;
					}
					else {
						errorState.fail("AudioService: Not enough available output channels on chosen device.");
						return false;
					}
				}
				else
					// There are enough channels
					outputChannelCount = settings.mOutputChannelCount;
			}

			if (inputChannelCount < 1 && outputChannelCount < 0)
			{
				errorState.fail("Cannot start audio stream with zero input and output channels.");
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

		
		int AudioService::getDeviceIndex(int hostApiIndex, int hostApiDeviceIndex)
		{
			return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, hostApiDeviceIndex);
		}
		
		
		bool AudioService::isActive()
		{
			return Pa_IsStreamActive(mStream) == 1;
		}
		
		
	}
	
}
