#pragma once

// std library includes
#include <vector>

// third party includes
#include <portaudio.h>

// nap includes
#include <utility/dllexport.h>
#include <nap/resource.h>

// audio includes
#include <audio/core/audionodemanager.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declares
        class AudioService;
        
        
        /*
         * Represents an audio stream handling multichannel audio input and output to and from an audio device.
         * It initializes an audio callback and owns a @NodeManager that performs the processing.
         */
        class NAPAPI AudioDevice : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            AudioDevice() = default;
            
            /**
             * Constructor used by the object factory
             */
            AudioDevice(AudioService& service);
            
            /**
             * Destructor stops the audio stream 
             */
            virtual ~AudioDevice();
            
            /** 
             * Starts the audio stream and handles possible errors
             */
            bool init(utility::ErrorState& errorState) override;
            
            /**
             * (Re)starts the audio stream and the audio processing
             * @return: true on success
             */
            bool start(utility::ErrorState& errorState);
            
            /** 
             * Stops the audio stream and the audio processing
             */
            void stop();
            
            /** 
             * Checks if the audio stream is running.
             */
            bool isActive();
            
            /** 
             * @return the node manager that takes care of the audio processing performed on this stream
             */
            NodeManager& getNodeManager();
            
        public:
            // PROPERTIES
            
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
            int mBufferSize = 64;

			/**
			 * The buffer size that is used internally by the node system to peform processing.
			 * This can be lower than mBufferSize but has to fit within mBufferSize a discrete amount of times.
			 * Lowering this can improve timing precision in the case that the node manager performs internal event scheduling, however will increase performance load.
			 */
			int mInternalBufferSize = 64;

        private:
            /*
             * Start the audio stream using the default audio devices available on the system.
             */
            bool startDefaultDevice(utility::ErrorState& errorState);
            
            PaStream* mStream = nullptr; // Pointer to the stream managed by portaudio.
            
            AudioService* mService = nullptr; // The audio service
        };
        
        
//        using AudioDeviceCreator = rtti::ObjectCreator<AudioDevice, AudioService>;
        
    }
    
}
