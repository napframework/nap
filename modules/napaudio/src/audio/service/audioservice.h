#pragma once

// third party includes
#include <portaudio.h>

// Nap includes
#include <nap/service.h>
#include <audio/core/audionodemanager.h>
#include <audio/utility/safeptr.h>
#include <utility/threading.h>

namespace nap
{    
    namespace audio
    {
		class AudioService;

		class NAPAPI AudioServiceConfiguration : public ServiceConfiguration
		{
			RTTI_ENABLE(ServiceConfiguration)
		public:
			virtual rtti::TypeInfo getServiceType() { return RTTI_OF(AudioService); }

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
            
            ~AudioService();
            
            /**
             * Register specific object creators
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
            NodeManager& getNodeManager();
            
            /**
             * Initializes portaudio.
             */
            bool init(nap::utility::ErrorState& errorState) override;
            
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
             * Returns information on all available host apis
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
             * @param deviceIndex: the number of the device
             */
            const PaDeviceInfo& getDeviceInfo(unsigned int hostApiIndex, unsigned int deviceIndex);
            
            /** 
             * Returns information on all the available devices
             * @param hostApiIndex: the number of the host api
             */
            std::vector<const PaDeviceInfo*> getDevices(unsigned int hostApiIndex);
            
            /** 
             * Prints the number and name of all available audio devices to the console
             */
            void printDevices();
            
            /** 
             * @return the name of an available device specified by host api and device number
             * @param hostApiIndex: the number of the host api
             * @param deviceIndex: the number of the devie
             */
            std::string getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex);
            
            /**
             * Returns the device index for a device specified by name for a given host API .
             * Uses case insensitive search.
             * Returns -1 if the device specified was not found.
             */
            int getDeviceIndex(int hostApiIndex, const std::string& device);
            
            /**
             * Returns the index for a certain host API specified both by name.
             * Uses case insensitive search.
             * Returns -1 if the host api specified was not found.
             */
            int getHostApiIndex(const std::string& hostApi);
            
            /**
             * Returns the index of the host API that is currently being used.
             */
            int getCurrentHostApiIndex() const { return mHostApiIndex; }
            
            /**
             * Returns the index of the current input device.
             */
            int getCurrentInputDeviceIndex() const { return mInputDeviceIndex; }
            
            /**
             * Returns the index of the current output device.
             */
            int getCurrentOutputDeviceIndex() const { return mOutputDeviceIndex; }

            /**
             * Stops the audio stream, waits for any running audio callback before returning.
             * @return true on success
             */
            bool stop() { return Pa_StopStream(mStream) == 0; }

            /**
             * Restart the audio stream after it has been stopped by calling @stop().
             * @return true on success
             */
            bool start() { return Pa_StartStream(mStream) == 0; }
            
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
             * Verifies if the ammounts of input and output channels specified in the configuration are supported on the given devices. If not and @mAllowChannelCountFailure is set to true, it will use the maximum numbers of channels of the selected devices instead. If @mAllowChannelCountFailure is false initialization will fail.
             */
            bool checkChannelCounts(int inputDeviceIndex, int outputDeviceIndex, utility::ErrorState& errorState);
            
            /*
             * Checks wether certain atomic types that are used within the library are lockfree and gives a warning if not.
             */
            void checkLockfreeTypes();
            
		private:
            NodeManager mNodeManager; // The node manager that performs the audio processing.
			PaStream* mStream = nullptr; // Pointer to the stream managed by portaudio.
            int mInputChannelCount = 1; // The actual input channel count can differ from the one in the configuration in case the configuration value is not supported by the device and mAllowChannelCountFailure is set to true. In this case the maximum amount of the device will be used.
            int mOutputChannelCount = 2; // The actual output channel count can differ from the one in the configuration in case the configuration value is not supported by the device and mAllowChannelCountFailure is set to true. In this case the maximum amount of the device will be used.
            int mHostApiIndex = -1; // The actual host Api being used.
            int mInputDeviceIndex = -1; // The actual input device being used, if any.
            int mOutputDeviceIndex = -1; // The actual output device being used, if any.
            
            // DeletionQueue with nodes that are no longer used and that can be cleared and destructed safely on the next audio callback.
            // Clearing is performed on the audio callback to make sure the node can not be destructed while it is being processed.
            DeletionQueue mDeletionQueue;
        };       
    }
}
