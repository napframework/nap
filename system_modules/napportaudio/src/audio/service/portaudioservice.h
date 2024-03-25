/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Audio includes
#include <audio/service/audioservice.h>
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
		class PortAudioService;
		
		
		class NAPAPI PortAudioServiceConfiguration : public ServiceConfiguration
		{
			RTTI_ENABLE(ServiceConfiguration)
			
		public:
			virtual rtti::TypeInfo getServiceType() const	{ return RTTI_OF(PortAudioService); }

            /**
             * Copyable struct containing audio device settings
             */
            struct NAPAPI DeviceSettings
            {
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
             * Settings of the audio device to initialize
             */
            DeviceSettings mDeviceSettings;

            /**
             * If this is set to true, the audio stream will start even if the number of channels specified in @mInputChannelCount and @mOutputChannelCount is not supported.
             * In this case a zero signal will be used to emulate the input from an unsupported input channel.
             */
            bool mAllowChannelCountFailure = true;

            /**
             * Indicates wether the app will continue to run when the audio device, samplerate and buffersize settings are invalid
             */
            bool mAllowDeviceFailure = true;
		};
		
		/**
		 * Service that provides audio input and output processing directly for hardware audio devices.
		 * Provides static methods to poll the current system for available audio devices using portaudio.
		 */
		class NAPAPI PortAudioService final : public Service
		{
		    RTTI_ENABLE(nap::Service)
		
		public:
			PortAudioService(ServiceConfiguration* configuration);
			
			~PortAudioService() = default;
			
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
              * Called before shutting down the services. Stops the running audio stream if any.
              */
             void preShutdown() override;

			/**
			 * @return the audio node manager owned by the audio service. The @NodeManager contains a node system that performs all the DSP.
			 */
			NodeManager& getNodeManager() { return mAudioService->getNodeManager(); }

			/**
			 * @return: returns wether we will allow input and output channel numbers that exceed the current device's maximum channel counts. If so zero signals will be returned for non-existing input channel numbers. If not initialization will fail.
			 */
			bool getAllowChannelCountFailure() { return getConfiguration<PortAudioServiceConfiguration>()->mAllowChannelCountFailure; }
			
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
             * Returns information of an audio device in a PaDeviceInfo struct defined by portaudio.
             * @param deviceIndex: the index of the device.
             * @return reference to a record containing information about the given device
             */
            const PaDeviceInfo& getDeviceInfo(unsigned int deviceIndex);

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
			 * Lookups the index for an input device with a certain name on a host API.
			 * @param hostApiIndex index of the host API
			 * @param device name of the input device
			 * @return the device index for a device specified by name for a given host API .
			 * Uses case insensitive search.
			 * Returns -1 if the device specified was not found.
			 */
			int getInputDeviceIndex(int hostApiIndex, const std::string& device);

            /**
             * Lookups the index for an output device with a certain name on a host API.
			 * @param hostApiIndex index of the host API
			 * @param device name of the output device
             * @return the device index for a device specified by name for a given host API .
             * Uses case insensitive search.
             * Returns -1 if the device specified was not found.
             */
            int getOutputDeviceIndex(int hostApiIndex, const std::string& device);

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
			int getCurrentBufferSize() const { return getConfiguration<PortAudioServiceConfiguration>()->mDeviceSettings.mBufferSize; }

            /**
             * Stores the device settings and tries to open audio stream with given device settings, return true on succes
             * The meesage from the errorState will be stored and can be retrieved using AudioService::getErrorMessage()
             * @param settings const ref to DeviceSettings struct
             * @param errorState contains any errors
             * @return true on success
             */
            bool openStream(const PortAudioServiceConfiguration::DeviceSettings& settings, utility::ErrorState& errorState);

            /**
             * Returns const ref to current device settings used by service
             * @return const ref to current device settings used by service
             */
            const PortAudioServiceConfiguration::DeviceSettings& getDeviceSettings() const;
			
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
             * @return Error message from last call to openStream() in case it was unsuccessful.
             */
            const std::string& getErrorMessage() const { return mErrorMessage; }

        private:
            /**
             * Does the actual work for the public openStream() so it can conveniently store the message from the errorState.
             * Stores the device settings and tries to open audio stream with given device settings, return true on succes
             * @param settings const ref to DeviceSettings struct
             * @param errorState contains any errors
             * @return true on success
             */
            bool _openStream(const PortAudioServiceConfiguration::DeviceSettings& settings, utility::ErrorState& errorState);

            /**
             * Tries to open the audio stream using the given settings.
             * @return true on success.
             */
            bool openStream(utility::ErrorState& errorState);

			/*
			 * Verifies if the ammounts of input and output channels specified in the configuration are supported on the given devices. If so, @inputDeviceIndex and @outputDeviceIndex will be set to these. If not and @mAllowChannelCountFailure is set to true, it will return the maximum numbers of channels of the selected devices instead. If @mAllowChannelCountFailure is false initialization will fail.
			 */
			bool checkChannelCounts(int inputDeviceIndex, int outputDeviceIndex, int& inputChannelCount,
			                        int& outputChannelCount, utility::ErrorState& errorState);
			
		private:
			PaStream* mStream = nullptr; // Pointer to the stream managed by portaudio.
			int mHostApiIndex = -1; // The actual host Api being used.
			int mInputDeviceIndex = -1; // The actual input device being used, if any.
			int mOutputDeviceIndex = -1; // The actual output device being used, if any.
			bool mPortAudioInitialized = false; // If port audio is initialized
			bool mMpg123Initialized	   = false;	// If mpg123 is initialized

            std::string mErrorMessage = ""; // Holds the error message if public openStream() method was called unsuccesfully.

            audio::AudioService* mAudioService = nullptr;
		};
	}
}
