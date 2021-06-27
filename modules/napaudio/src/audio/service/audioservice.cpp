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


RTTI_BEGIN_CLASS(nap::audio::AudioServiceConfiguration)
	RTTI_PROPERTY("HostApi", &nap::audio::AudioServiceConfiguration::mHostApi,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputDevice", &nap::audio::AudioServiceConfiguration::mInputDevice,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputDevice", &nap::audio::AudioServiceConfiguration::mOutputDevice,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InputChannelCount", &nap::audio::AudioServiceConfiguration::mInputChannelCount,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputChannelCount", &nap::audio::AudioServiceConfiguration::mOutputChannelCount,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AllowChannelCountFailure", &nap::audio::AudioServiceConfiguration::mAllowChannelCountFailure,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AllowDeviceFailure", &nap::audio::AudioServiceConfiguration::mAllowDeviceFailure,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DisableInput", &nap::audio::AudioServiceConfiguration::mDisableInput,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DisableOutput", &nap::audio::AudioServiceConfiguration::mDisableOutput,
					  nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleRate", &nap::audio::AudioServiceConfiguration::mSampleRate,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize", &nap::audio::AudioServiceConfiguration::mBufferSize,
		              nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("InternalBufferSize", &nap::audio::AudioServiceConfiguration::mInternalBufferSize,
		              nap::rtti::EPropertyMetaData::Default)
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

			AudioServiceConfiguration* configuration = getConfiguration<AudioServiceConfiguration>();
			int inputDeviceIndex = -1;
			int outputDeviceIndex = -1;
			int inputChannelCount = 0;
			int outputChannelCount = 0;
			
			// Initialize the portaudio library
			PaError error = Pa_Initialize();
			if (!errorState.check(error == paNoError, "Portaudio error: %s", Pa_GetErrorText(error)))
				return false;

			mPortAudioInitialized = true;
			Logger::info("Portaudio initialized");
			printDevices();
			
			// Initialize the audio device
			if (configuration->mBufferSize % configuration->mInternalBufferSize != 0) {
				errorState.fail("AudioService: Internal buffer size does not fit device buffer size");
				return false;
			}
			
			if (configuration->mHostApi.empty())
				mHostApiIndex = Pa_GetDefaultHostApi();
			else
				mHostApiIndex = getHostApiIndex(configuration->mHostApi);
			if (mHostApiIndex < 0)
			{
				if (!configuration->mAllowDeviceFailure)
				{
					errorState.fail("Audio host API not found: %s", configuration->mHostApi.c_str());
					return false;
				}
				else {
					Logger::warn("Audio host API not found: %s", configuration->mHostApi.c_str());
					return true;
				}
			}
			
			if (configuration->mDisableInput)
			{
				inputDeviceIndex = -1;
			}
			else {
				if (configuration->mInputDevice.empty())
					inputDeviceIndex = Pa_GetDefaultInputDevice();
				else
					inputDeviceIndex = getDeviceIndex(mHostApiIndex, configuration->mInputDevice);
				if (inputDeviceIndex < 0) {
					if (!configuration->mAllowDeviceFailure)
					{
						errorState.fail("Audio input device not found: %s", configuration->mInputDevice.c_str());
						return false;
					} else
						Logger::info("Audio input device not found: %s", configuration->mInputDevice.c_str());
				}
			}

			if (configuration->mDisableOutput)
			{
				outputDeviceIndex = -1;
			}
			else {
				if (configuration->mOutputDevice.empty())
					outputDeviceIndex = Pa_GetDefaultOutputDevice();
				else
					outputDeviceIndex = getDeviceIndex(mHostApiIndex, configuration->mOutputDevice);
				if (outputDeviceIndex < 0)
				{
					if (!configuration->mAllowDeviceFailure)
					{
						errorState.fail("Audio output device not found: %s", configuration->mOutputDevice.c_str());
						return false;
					}
					else
						Logger::info("Audio output device not found: %s", configuration->mOutputDevice.c_str());
				}
			}

			if (inputDeviceIndex < 0 && outputDeviceIndex < 0)
			{
				if (!configuration->mAllowDeviceFailure)
				{
					errorState.fail("Cannot start audio stream with neither input nor output.");
					return false;
				}
				else {
					Logger::warn("Cannot start audio stream with neither input nor output.");
					return true;
				}
			}

			if (!checkChannelCounts(inputDeviceIndex, outputDeviceIndex, inputChannelCount, outputChannelCount, errorState))
				return false;

			if (!(openStream(mHostApiIndex, inputDeviceIndex, outputDeviceIndex, inputChannelCount, outputChannelCount, configuration->mSampleRate, configuration->mBufferSize, configuration->mInternalBufferSize, errorState) && start(errorState)))
			{
				if (!configuration->mAllowDeviceFailure)
				{
					errorState.fail(
							"Portaudio stream failed to start with: input: %s (%i channels), output: %s (%i channels), samplerate %i, buffersize %i",
							inputDeviceIndex >= 0 ? Pa_GetDeviceInfo(inputDeviceIndex)->name : "no input device", inputChannelCount,
							outputDeviceIndex >= 0 ? Pa_GetDeviceInfo(outputDeviceIndex)->name : "no output device", outputChannelCount, int(configuration->mSampleRate),
							configuration->mBufferSize);
					return false;
				}
				else {
					Logger::info(
						"Portaudio stream failed to start with: input: %s (%i channels), output: %s (%i channels), samplerate %i, buffersize %i",
						inputDeviceIndex >= 0 ? Pa_GetDeviceInfo(inputDeviceIndex)->name : "no input device", inputChannelCount,
						outputDeviceIndex >= 0 ? Pa_GetDeviceInfo(outputDeviceIndex)->name : "no output device", outputChannelCount, int(configuration->mSampleRate),
						configuration->mBufferSize);
				}
				return true;
			}

			// Log portaudio stream settings
			Logger::info("Portaudio stream started:");
			if (inputDeviceIndex >= 0)
				Logger::info("Input device: %s, %i channel(s)", Pa_GetDeviceInfo(inputDeviceIndex)->name, mNodeManager.getInputChannelCount());
			else
				Logger::info("No input device");

			if (outputDeviceIndex >= 0)
				Logger::info("Output device: %s, %i channel(s)", Pa_GetDeviceInfo(outputDeviceIndex)->name, mNodeManager.getOutputChannelCount());
			else
				Logger::info("No output device");
			Logger::info("Samplerate: %i", int(mNodeManager.getSampleRate()));
			Logger::info("Buffersize: %i", mBufferSize);

			return true;
		}


		void AudioService::preShutdown()
		{
			// Stop the running stream to avoid problems destroying active audio objects
			if (mPortAudioInitialized)
				Pa_StopStream(mStream);
		}


		void AudioService::shutdown()
		{
			// First close port-audio, only do so when initialized
			if (mPortAudioInitialized)
			{
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


		bool AudioService::openStream(int hostApi, int inputDeviceIndex, int outputDeviceIndex, int inputChannelCount, int outputChannelCount, float sampleRate, int bufferSize, int internalBufferSize, utility::ErrorState& errorState)
		{
			// The stream can only be opened when it's closed
			assert(mStream == nullptr);

			if (inputChannelCount != mNodeManager.getInputChannelCount())
				mNodeManager.setInputChannelCount(inputChannelCount);
			if (outputChannelCount != mNodeManager.getOutputChannelCount())
				mNodeManager.setOutputChannelCount(outputChannelCount);
			if (sampleRate != mNodeManager.getSampleRate())
				mNodeManager.setSampleRate(sampleRate);
			if (internalBufferSize != mNodeManager.getInternalBufferSize())
				mNodeManager.setInternalBufferSize(internalBufferSize);

			mHostApiIndex = hostApi;
			mInputDeviceIndex = inputDeviceIndex;
			mOutputDeviceIndex = outputDeviceIndex;
			mBufferSize = bufferSize;

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

			PaError error = Pa_OpenStream(&mStream, inputParamsPtr, outputParamsPtr, mNodeManager.getSampleRate(), mBufferSize, paNoFlag, &audioCallback, this);
			if (error != paNoError)
			{
				errorState.fail("Error opening audio stream: %s", Pa_GetErrorText(error));
				mInputDeviceIndex = -1;
				mOutputDeviceIndex = -1;
				mStream = nullptr;

				saveConfiguration();
				return false;
			}

			saveConfiguration();
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
			
			const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(inputDeviceIndex);
			const PaDeviceInfo* outputDeviceInfo = Pa_GetDeviceInfo(outputDeviceIndex);
			
			if (inputDeviceInfo == nullptr)
			{
				// There is no input device
				if (configuration->mDisableInput == false && configuration->mInputChannelCount > 0)
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
				if (configuration->mInputChannelCount > inputDeviceInfo->maxInputChannels)
				{
					// There are less channels than requested
					if (configuration->mAllowChannelCountFailure)
					{
						Logger::warn(
								"AudioService: Requested number of %i input channels not available, initializing with only %i",
								configuration->mInputChannelCount, inputDeviceInfo->maxInputChannels);
						inputChannelCount = inputDeviceInfo->maxInputChannels;
					}
					else {
						errorState.fail("AudioService: Not enough available input channels on chosen device.");
						return false;
					}
				}
				else
					// There are enough channels
					inputChannelCount = configuration->mInputChannelCount;
			}
			
			
			if (!outputDeviceInfo)
			{
				// There is no output device
				if (configuration->mDisableOutput == false && configuration->mOutputChannelCount > 0)
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
				if (configuration->mOutputChannelCount > outputDeviceInfo->maxOutputChannels)
				{
					// There are less channels than requested
					if (configuration->mAllowChannelCountFailure)
					{
						Logger::warn(
								"AudioService: Requested number of %i output channels not available, initializing with only %i",
								configuration->mOutputChannelCount, outputDeviceInfo->maxOutputChannels);
						outputChannelCount = outputDeviceInfo->maxOutputChannels;
					}
					else {
						errorState.fail("AudioService: Not enough available output channels on chosen device.");
						return false;
					}
				}
				else
					// There are enough channels
					outputChannelCount = configuration->mOutputChannelCount;
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
		
		
		void AudioService::saveConfiguration()
		{
			auto configuration = getConfiguration<AudioServiceConfiguration>();
			configuration->mSampleRate = mNodeManager.getSampleRate();
			configuration->mBufferSize = mBufferSize;
			configuration->mInternalBufferSize = mNodeManager.getInternalBufferSize();
			configuration->mInputChannelCount = mNodeManager.getInputChannelCount();
			configuration->mOutputChannelCount = mNodeManager.getOutputChannelCount();
			configuration->mHostApi = getHostApiName(mHostApiIndex);
			configuration->mInputDevice = mInputDeviceIndex > -1 ? Pa_GetDeviceInfo(mInputDeviceIndex)->name : "";
			configuration->mOutputDevice = mOutputDeviceIndex > -1 ? Pa_GetDeviceInfo(mOutputDeviceIndex)->name : "";
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
