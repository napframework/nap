#pragma once

// third party includes
#include <portaudio.h>

// Nap includes
#include <nap/service.h>
#include <audio/core/audionodemanager.h>

namespace nap
{    
    namespace audio
    {
		class AudioService;

		class NAPAPI AudioServiceConfiguration : public ServiceConfiguration
		{
			RTTI_ENABLE(ServiceConfiguration)
		public:
			virtual rtti::TypeInfo GetServiceType() { return RTTI_OF(AudioService); }

            /** 
             * If true, the default host API, audio input and output device on this system are being used
             */
            bool mUseDefaultDevice = true;
            
            /**
             * Name of the host API (or driver type) used for this audio stream. Use @AudioService to poll for available host APIs
             * The host API is an audio driver API like Windows MME, ASIO, CoreAudio, Jack, etc.
             */
            std::string mHostApi = "";
            
            /**
             * Name of the input device being used. Use @AudioService to poll for available devices for a certain host API.
             */
            std::string mInputDevice = "";

            /** 
             * Name of the output device being used. Use @AudioService to poll for available devices for a certain host API.
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
             * The sample rate the audio stream will run on, the number of samples processed per channel per second.
             */
            float mSampleRate = 44100;
            
            /**
             * The buffer size the audio stream will run on, every audio callback processes this amount of samples per channel
             */
            int mBufferSize = 256;

			/**
			 * The buffer size that is used internally by the node system to peform processing.
			 * This can be lower than mBufferSize but has to fit within mBufferSize a discrete amount of times.
			 * Lowering this can improve timing precision in the case that the node manager performs internal event scheduling, however will increase performance load.
			 */
			int mInternalBufferSize = 256;
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
			 *	Shutdown portaudio
			 */
			void shutdown() override;

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
             * Returns the device index for a device for a certain host API specified both by name.
             * Uses case insensitive search.
             * Returns -1 if the device specified was not found.
             */
            int getDeviceIndex(const std::string& hostApi, const std::string& device);
            
		private:
			/*
             * Start the audio stream using the default audio devices available on the system.
             */
            bool startDefaultDevice(utility::ErrorState& errorState);
		
		private:
            NodeManager mNodeManager; // The node manager that performs the audio processing.
			PaStream* mStream = nullptr; // Pointer to the stream managed by portaudio.
        };       
    }
}
