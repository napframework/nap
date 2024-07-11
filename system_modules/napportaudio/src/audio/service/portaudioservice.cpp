/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portaudioservice.h"

// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>
#include <nap/core.h>

RTTI_BEGIN_STRUCT(nap::audio::PortAudioServiceConfiguration::DeviceSettings, "Audio device settings")
        RTTI_PROPERTY("HostApi", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mHostApi, nap::rtti::EPropertyMetaData::Default, "Name of the host API (or driver type) used for the audio stream")
        RTTI_PROPERTY("InputDevice", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mInputDevice, nap::rtti::EPropertyMetaData::Default, "Name of the audio input device")
        RTTI_PROPERTY("OutputDevice", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mOutputDevice, nap::rtti::EPropertyMetaData::Default, "Name of the audio output device")
        RTTI_PROPERTY("InputChannelCount", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mInputChannelCount, nap::rtti::EPropertyMetaData::Default, "Number of stream input channels to use")
        RTTI_PROPERTY("OutputChannelCount", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mOutputChannelCount, nap::rtti::EPropertyMetaData::Default, "Number of stream output channels to use")
        RTTI_PROPERTY("DisableInput", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mDisableInput, nap::rtti::EPropertyMetaData::Default, "Disables audio input")
        RTTI_PROPERTY("DisableOutput", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mDisableOutput, nap::rtti::EPropertyMetaData::Default, "Disables audio output")
        RTTI_PROPERTY("SampleRate", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mSampleRate, nap::rtti::EPropertyMetaData::Default, "Audio sample rate, the number of samples processed per channel per second")
        RTTI_PROPERTY("BufferSize", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mBufferSize, nap::rtti::EPropertyMetaData::Default, "Audio buffer size, the number of samples to process every click. Lower values = less latency")
        RTTI_PROPERTY("InternalBufferSize", &nap::audio::PortAudioServiceConfiguration::DeviceSettings::mInternalBufferSize, nap::rtti::EPropertyMetaData::Default, "The buffer size used by the node system for processing. Lower values = less latency")
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::audio::PortAudioServiceConfiguration, "Audio service configuration")
        RTTI_PROPERTY("Audio Device Settings", &nap::audio::PortAudioServiceConfiguration::mDeviceSettings, nap::rtti::EPropertyMetaData::Default, "Audio device settings")
        RTTI_PROPERTY("AllowChannelCountFailure", &nap::audio::PortAudioServiceConfiguration::mAllowChannelCountFailure, nap::rtti::EPropertyMetaData::Default, "Start the audio stream when the number of input or output channels is not supported")
        RTTI_PROPERTY("AllowDeviceFailure", &nap::audio::PortAudioServiceConfiguration::mAllowDeviceFailure, nap::rtti::EPropertyMetaData::Default, "Continue initialization when audio device settings are not supported or invalid")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::PortAudioService, "Audio input & output device manager")
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
		
		
		PortAudioService::PortAudioService(ServiceConfiguration* configuration) :
				Service(configuration)
		{
		}
		
		
		void PortAudioService::registerObjectCreators(rtti::Factory& factory)
		{
		}
		
		
		bool PortAudioService::init(nap::utility::ErrorState& errorState)
		{
			auto* configuration = getConfiguration<PortAudioServiceConfiguration>();
            const auto& device_settings = configuration->mDeviceSettings;

            mAudioService = getCore().getService<audio::AudioService>();

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
				Logger::info("Input device: %s, %i channel(s)", Pa_GetDeviceInfo(mInputDeviceIndex)->name, getNodeManager().getInputChannelCount());
			else
				Logger::info("No input device");

			if (mOutputDeviceIndex >= 0)
				Logger::info("Output device: %s, %i channel(s)", Pa_GetDeviceInfo(mOutputDeviceIndex)->name, getNodeManager().getOutputChannelCount());
			else
				Logger::info("No output device");
			Logger::info("Samplerate: %i", int(getNodeManager().getSampleRate()));
			Logger::info("Buffersize: %i", device_settings.mBufferSize);

			return true;
		}


        bool PortAudioService::openStream(const PortAudioServiceConfiguration::DeviceSettings& deviceSettings, utility::ErrorState& errorState)
        {
            auto result = _openStream(deviceSettings, errorState);
            mErrorMessage = errorState.toString();
            return result;
        }


        bool PortAudioService::_openStream(const PortAudioServiceConfiguration::DeviceSettings& deviceSettings, utility::ErrorState& errorState)
        {
            // copy settings to configuration
            auto* configuration = getConfiguration<PortAudioServiceConfiguration>();
            configuration->mDeviceSettings = deviceSettings;

            // close stream
            if(isOpened())
            {
                if(!closeStream(errorState))
                    return false;
            }

            int inputDeviceIndex = -1;
            int outputDeviceIndex = -1;
            int inputChannelCount = deviceSettings.mInputChannelCount;
            int outputChannelCount = deviceSettings.mOutputChannelCount;

            // Initialize the audio device
            if (deviceSettings.mBufferSize % deviceSettings.mInternalBufferSize != 0) {
                errorState.fail("PortAudioService: Internal buffer size does not fit device buffer size");
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

            if (deviceSettings.mDisableInput || inputChannelCount < 1)
            {
                inputDeviceIndex = -1;
            }
            else
            {
                inputDeviceIndex = deviceSettings.mInputDevice.empty() ? Pa_GetDefaultInputDevice() : getInputDeviceIndex(mHostApiIndex, deviceSettings.mInputDevice);
                if (inputDeviceIndex < 0)
                {
                    errorState.fail("Audio input device not found: %s", deviceSettings.mInputDevice.c_str());
                    return false;
                }
            }

            if (deviceSettings.mDisableOutput || outputChannelCount < 1)
            {
                outputDeviceIndex = -1;
            }
            else
            {
                outputDeviceIndex = deviceSettings.mOutputDevice.empty() ? Pa_GetDefaultOutputDevice() : getOutputDeviceIndex(mHostApiIndex, deviceSettings.mOutputDevice);
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


        const PortAudioServiceConfiguration::DeviceSettings& PortAudioService::getDeviceSettings() const
        {
            auto* configuration = getConfiguration<PortAudioServiceConfiguration>();
            return configuration->mDeviceSettings;
        }


		void PortAudioService::shutdown()
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
		}


        void PortAudioService::preShutdown()
        {
            // Stop the running stream to avoid problems destroying active audio objects
            if (mPortAudioInitialized)
                Pa_StopStream(mStream);
        }


        bool PortAudioService::openStream(utility::ErrorState& errorState)
        {
            // The stream can only be opened when it's closed
            assert(mStream == nullptr);

            const auto& device_settings = getConfiguration<PortAudioServiceConfiguration>()->mDeviceSettings;

            getNodeManager().setInputChannelCount(device_settings.mDisableInput ? 0 : device_settings.mInputChannelCount);
            getNodeManager().setOutputChannelCount(device_settings.mDisableOutput ? 0 : device_settings.mOutputChannelCount);
            getNodeManager().setSampleRate(device_settings.mSampleRate);
            getNodeManager().setInternalBufferSize(device_settings.mInternalBufferSize);

            PaStreamParameters inputParameters;
            inputParameters.device = mInputDeviceIndex;
            inputParameters.channelCount = getNodeManager().getInputChannelCount();
            inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            inputParameters.hostApiSpecificStreamInfo = nullptr;

            PaStreamParameters outputParameters;
            outputParameters.device = mOutputDeviceIndex;
            outputParameters.channelCount = getNodeManager().getOutputChannelCount();
            outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
            outputParameters.hostApiSpecificStreamInfo = nullptr;

            PaStreamParameters* inputParamsPtr = nullptr;
            if (mInputDeviceIndex >= 0)
            {
#ifdef __APPLE__
                inputParameters.suggestedLatency = 0.f;
#else
                inputParameters.suggestedLatency = Pa_GetDeviceInfo(mInputDeviceIndex)->defaultLowInputLatency;
#endif
                inputParamsPtr = &inputParameters;
            }
            PaStreamParameters* outputParamsPtr = nullptr;
            if (mOutputDeviceIndex >= 0)
            {
#ifdef __APPLE__
                outputParameters.suggestedLatency = 0.f;
#else
                outputParameters.suggestedLatency = Pa_GetDeviceInfo(mOutputDeviceIndex)->defaultLowOutputLatency;
#endif
                outputParamsPtr = &outputParameters;
            }

            PaError error = Pa_OpenStream(&mStream, inputParamsPtr, outputParamsPtr, getNodeManager().getSampleRate(), device_settings.mBufferSize, paNoFlag, &audioCallback, mAudioService);
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
		
		
		bool PortAudioService::closeStream(utility::ErrorState& errorState)
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
		
		
		bool PortAudioService::stop(utility::ErrorState& errorState)
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
		
		
		bool PortAudioService::start(utility::ErrorState& errorState)
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
		
		
		unsigned int PortAudioService::getHostApiCount()
		{
			return Pa_GetHostApiCount();
		}
		
		
		const PaHostApiInfo& PortAudioService::getHostApiInfo(unsigned int hostApiIndex)
		{
			return *Pa_GetHostApiInfo(hostApiIndex);
		}
		
		
		std::vector<const PaHostApiInfo*> PortAudioService::getHostApis()
		{
			std::vector<const PaHostApiInfo*> result;
			for (auto i = 0; i < Pa_GetHostApiCount(); ++i)
			{
				result.emplace_back(&getHostApiInfo(i));
			}
			return result;
		}
		
		
		std::string PortAudioService::getHostApiName(unsigned int hostApiIndex)
		{
			assert(hostApiIndex < getHostApiCount());
			return getHostApiInfo(hostApiIndex).name;
		}
		
		
		unsigned int PortAudioService::getDeviceCount(unsigned int hostApiIndex)
		{
			return getHostApiInfo(hostApiIndex).deviceCount;
		}
		
		
		const PaDeviceInfo& PortAudioService::getDeviceInfo(unsigned int hostApiIndex, unsigned int deviceIndex)
		{
			return *Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex));
		}


        const PaDeviceInfo& PortAudioService::getDeviceInfo(unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(deviceIndex);
        }


        std::vector<const PaDeviceInfo*> PortAudioService::getDevices(unsigned int hostApiIndex)
		{
			std::vector<const PaDeviceInfo*> result;
			for (auto i = 0; i < getHostApiInfo(hostApiIndex).deviceCount; ++i)
			{
				result.emplace_back(&getDeviceInfo(hostApiIndex, i));
			}
			return result;
		}
		
		
		void PortAudioService::printDevices()
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
		
		
		std::string PortAudioService::getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex)
		{
			assert(hostApiIndex < getHostApiCount());
			assert(deviceIndex < getDeviceCount(hostApiIndex));
			return getDeviceInfo(hostApiIndex, deviceIndex).name;
		}
		
		
		int PortAudioService::getInputDeviceIndex(int hostApiIndex, const std::string& device)
		{
			for (auto deviceIndex = 0; deviceIndex < getHostApiInfo(hostApiIndex).deviceCount; ++deviceIndex)
			{
				auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
				if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name) && deviceInfo.maxInputChannels > 0)
					return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
			}
			
			return -1;
		}


        int PortAudioService::getOutputDeviceIndex(int hostApiIndex, const std::string& device)
        {
            for (auto deviceIndex = 0; deviceIndex < getHostApiInfo(hostApiIndex).deviceCount; ++deviceIndex)
            {
                auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
                if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name) && deviceInfo.maxOutputChannels > 0)
                    return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
            }

            return -1;
        }


        int PortAudioService::getHostApiIndex(const std::string& hostApi)
		{
			auto hostApiIndex = -1;
			
			for (auto i = 0; i < getHostApiCount(); ++i)
				if (nap::utility::toLower(hostApi) == nap::utility::toLower(getHostApiInfo(i).name))
					hostApiIndex = i;
			
			return hostApiIndex;
		}
		
		
		bool PortAudioService::checkChannelCounts(int inputDeviceIndex, int outputDeviceIndex, int& inputChannelCount,
		                                      int& outputChannelCount, utility::ErrorState& errorState)
		{
			PortAudioServiceConfiguration* configuration = getConfiguration<PortAudioServiceConfiguration>();
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
						Logger::warn("PortAudioService: input device not found, initializing without input channels.");
						inputChannelCount = 0;
					}
					else {
						errorState.fail("PortAudioService: input device not found.");
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
								"PortAudioService: Requested number of %i input channels not available, initializing with only %i",
                                settings.mInputChannelCount, inputDeviceInfo->maxInputChannels);
						inputChannelCount = inputDeviceInfo->maxInputChannels;
					}
					else {
						errorState.fail("PortAudioService: Not enough available input channels on chosen device.");
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
						Logger::warn("PortAudioService: output device not found, initializing without output channels.");
						outputChannelCount = 0;
					}
					else {
						errorState.fail("PortAudioService: output device not found.");
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
								"PortAudioService: Requested number of %i output channels not available, initializing with only %i",
                                settings.mOutputChannelCount, outputDeviceInfo->maxOutputChannels);
						outputChannelCount = outputDeviceInfo->maxOutputChannels;
					}
					else {
						errorState.fail("PortAudioService: Not enough available output channels on chosen device.");
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
		

		int PortAudioService::getDeviceIndex(int hostApiIndex, int hostApiDeviceIndex)
		{
			return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, hostApiDeviceIndex);
		}
		
		
		bool PortAudioService::isActive()
		{
			return Pa_IsStreamActive(mStream) == 1;
		}
		
		
	}
	
}
