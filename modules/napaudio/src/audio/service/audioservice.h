/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/utility/safeptr.h>

// Nap includes
#include <nap/service.h>
#include <utility/threading.h>

// third party includes
#include <portaudio.h>


namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class AudioService;
		
		
		class NAPAPI AudioServiceConfiguration : public ServiceConfiguration
		{
			RTTI_ENABLE(ServiceConfiguration)
			
		public:
			virtual rtti::TypeInfo getServiceType()
			{ return RTTI_OF(AudioService); }
			
			/**
			 * Name of the host API (or driver type) used for this audio stream. Use @AudioService to poll for available host APIs
			 * The host API is an audio driver API like Windows MME, ASIO, CoreAudio, Jack, etc.
			 * If left empty the default host API will be used.
			 */
			std::string mHostApi = "";
			
			/**
			 * Name of the input device being used. Use @AudioService to poll for available devices for a certain host API.
			 * If left empty, the default input device will be used.
			 */
			std::string mInputDevice = "";
			
			/**
			 * Name of the output device being used. Use @AudioService to poll for available devices for a certain host API.
			 * If left empty the default output device will be used.
			 */
			std::string mOutputDevice = "";
			
			/**
			 * The number of input channels in the stream.
			 * If the chosen device @mInputDevice does not support this amount of channels the stream will not start.
			 */
			int mInputChannelCount = 1;
			
			/**
			 * The number of output channels in the stream.
			 * If the chosen device @mOutputDevice does not support this amount of channels the stream will not start.
			 */
			int mOutputChannelCount = 2;
			
			/**
			 * If this is set to true, the audio stream will start even if the number of channels specified in @mInputChannelCount and @mOutputChannelCount is not supported.
			 * In this case a zero signal will be used to emulate the input from an unsupported input channel.
			 */
			bool mAllowChannelCountFailure = true;
			
			/**
			 * Indicates wether the app will continue to run when the audio device, samplerate and buffersize settings are invalid
			 */
			bool mAllowDeviceFailure = true;
			
			/**
			 * If set to true the audio will start with only an output device.
			 */
			bool mDisableInput = false;

			/**
			 * If set to true the audio will start with only an input device.
			 */
			bool mDisableOutput = false;
			
			/**
			 * The sample rate the audio stream will run on, the number of samples processed per channel per second.
			 */
			float mSampleRate = 44100;
			
			/**
			 * The buffer size the audio stream will run on, every audio callback processes this amount of samples per channel
			 */
			int mBufferSize = 1024;
			
			/**
			 * The buffer size that is used internally by the node system to peform processing.
			 * This can be lower than mBufferSize but has to fit within mBufferSize a discrete amount of times.
			 * Lowering this can improve timing precision in the case that the node manager performs internal event scheduling, however will increase performance load.
			 */
			int mInternalBufferSize = 1024;
		};
		
		/**
		 * Service that provides audio input and output processing directly for hardware audio devices.
		 * Provides static methods to poll the current system for available audio devices using portaudio.
		 */
		class NAPAPI AudioService final : public Service
		{
		RTTI_ENABLE(nap::Service)
		
		public:
			AudioService(ServiceConfiguration* configuration);
			
			~AudioService() = default;
			
			/**
			 * Register specific object creators
			 */
			void registerObjectCreators(rtti::Factory& factory) override;
			
			/**
			 * Initializes portaudio.
			 */
			bool init(nap::utility::ErrorState& errorState) override;

			/**
			 * Called on shutdown of the service. Closes portaudio stream and shuts down portaudio.
			 */
			 void shutdown() override;

			/**
			 * @return the audio node manager owned by the audio service. The @NodeManager contains a node system that performs all the DSP.
			 */
			NodeManager& getNodeManager();

			/**
			 * @return: returns wether we will allow input and output channel numbers that exceed the current device's maximum channel counts. If so zero signals will be returned for non-existing input channel numbers. If not initialization will fail.
			 */
			bool getAllowChannelCountFailure() { return getConfiguration<AudioServiceConfiguration>()->mAllowChannelCountFailure; }
			
			/**
			 * @return: the number of available host APIs ont this system
			 */
			unsigned int getHostApiCount();
			
			/**
			 * Returns information about a given host api
			 * @param hostApiIndex:
			 * @return: struct containing information about the specified host api
			 */
			const PaHostApiInfo& getHostApiInfo(unsigned int hostApiIndex);
			
			/**
			 * @return information on all available host apis
			 */
			std::vector<const PaHostApiInfo*> getHostApis();
			
			/**
			 * @return: name of the specified host API
			 */
			std::string getHostApiName(unsigned int hostApiIndex);
			
			/**
			 * @param hostApiIndex: the number of the host api
			 * @return: the number of all available audio devices for a certain host api, the total number contains both input and output devices separately.
			 */
			unsigned int getDeviceCount(unsigned int hostApiIndex);
			
			/**
			 * Returns information of an audio device in a PaDeviceInfo struct defined by portaudio.
			 * @param hostApiIndex: the number of the host api
			 * @param localDeviceIndex: the number of the device counting from 0 to the number of devices belonging to this host api.
			 * @return reference to a record containing information about the given device
			 */
			const PaDeviceInfo& getDeviceInfo(unsigned int hostApiIndex, unsigned int localDeviceIndex);
			
			/**
			 * Returns information on all the available devices for a given host API
			 * @param hostApiIndex: the number of the host api
			 * @return vector with pointers to records containing information about each device.
			 */
			std::vector<const PaDeviceInfo*> getDevices(unsigned int hostApiIndex);
			
			/**
			 * Prints the number and name of all available audio devices to the console
			 */
			void printDevices();
			
			/**
			 * @param hostApiIndex: the number of the host api
			 * @param deviceIndex: the number of the devie within the host api
			 * @return the name of an available device specified by host api and device number
			 */
			std::string getDeviceName(unsigned int hostApiIndex, unsigned int localDeviceIndex);
			
			/**
			 * Lookups the index for a device with a certain name on a host API.
			 * @return the device index for a device specified by name for a given host API .
			 * Uses case insensitive search.
			 * Returns -1 if the device specified was not found.
			 */
			int getDeviceIndex(int hostApiIndex, const std::string& device);
			
			/**
			 * Returns the device index for a device specified by a local index in the list of devices for a specific host API.
			 * Returns -1 if the specified device was not found.
			 */
			int getDeviceIndex(int hostApiIndex, int hostApiDeviceIndex);
			
			/**
			 * Seatch for the index of a host API by name.
			 * Uses case insensitive search.
			 * @return the index for a certain host API specified both by name.
			 * Returns -1 if the host api specified was not found.
			 */
			int getHostApiIndex(const std::string& hostApi);
			
			/**
			 * @return the index of the host API that is currently being used.
			 */
			int getCurrentHostApiIndex() const { return mHostApiIndex; }
			
			/**
			 * @return the index of the current input device.
			 */
			int getCurrentInputDeviceIndex() const { return mInputDeviceIndex; }
			
			/**
			 * @return the index of the current output device.
			 */
			int getCurrentOutputDeviceIndex() const { return mOutputDeviceIndex; }
			
			/**
			 * @return the current buffer size.
			 */
			int getCurrentBufferSize() const { return mBufferSize; }
			
			/**
			 * Tries to open the audio stream using the given settings.
			 * @return true on success.
			 */
			bool openStream(int hostApi, int inputDeviceIndex, int outputDeviceIndex, int inputChannelCount, int outputChannelCount, float sampleRate, int bufferSize, int internalBufferSize, utility::ErrorState& errorState);
			
			/**
			 * Closes the current stream. Assumes that it has been opened successfully.
			 * @return true on success.
			 */
			bool closeStream(utility::ErrorState& errorState);
			
			/**
			 * Restart the audio stream after it has been stopped by calling @stop().
			 * Logs errors in the @errorState. Assumes the stream has been opened succesfully.
			 * @return true on success
			 */
			bool start(utility::ErrorState& errorState);
			
			/**
			 * Stops the audio stream, waits for any running audio callback before returning.
			 * Logs errors in the @errorState. Assumes the stream has been opened succesfully.
			 * @return true on success
			 */
			bool stop(utility::ErrorState& errorState);
			
			/**
			 * @return Whether the audio stream is succesfully initialized
			 */
			bool isOpened() { return mStream != nullptr; }
			
			/**
			 * @return Whether the audio stream is currently running and not been paused.
			 */
			bool isActive();
			
			/**
             * This function is typically called by a hardware callback from the device to perform all the audio processing.
             * It performs memory management and processes a lockfree event queue before it invokes the @NodeManager::process() to process audio.
             * @param inputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param outputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param framesPerBuffer: the number of samples that has to be processed per channel
             */
			void onAudioCallback(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer);
			
			/**
			 * Enqueue a task to be executed within the process() method for thread safety
			 */
			void enqueueTask(TaskQueue::Task task) { mNodeManager.enqueueTask(task); }
		
		private:
			/*
			 * Verifies if the ammounts of input and output channels specified in the configuration are supported on the given devices. If so, @inputDeviceIndex and @outputDeviceIndex will be set to these. If not and @mAllowChannelCountFailure is set to true, it will return the maximum numbers of channels of the selected devices instead. If @mAllowChannelCountFailure is false initialization will fail.
			 */
			bool checkChannelCounts(int inputDeviceIndex, int outputDeviceIndex, int& inputChannelCount,
			                        int& outputChannelCount, utility::ErrorState& errorState);
			
			/*
			 * Checks wether certain atomic types that are used within the library are lockfree and gives a warning if not.
			 */
			void checkLockfreeTypes();
			
			/*
			 * Copies the current settings to the configuration object.
			 */
			void saveConfiguration();
		
		private:
			NodeManager mNodeManager; // The node manager that performs the audio processing.
			PaStream* mStream = nullptr; // Pointer to the stream managed by portaudio.
			int mHostApiIndex = -1; // The actual host Api being used.
			int mInputDeviceIndex = -1; // The actual input device being used, if any.
			int mOutputDeviceIndex = -1; // The actual output device being used, if any.
			int mBufferSize = 1024; // The actual buffersize that the audio device runs on
			bool mPortAudioInitialized = false; // If port audio is initialized
			bool mMpg123Initialized	   = false;	// If mpg123 is initialized

			// DeletionQueue with nodes that are no longer used and that can be cleared and destructed safely on the next audio callback.
			// Clearing is performed on the audio callback to make sure the node can not be destructed while it is being processed.
			DeletionQueue mDeletionQueue;
		};
	}
}
