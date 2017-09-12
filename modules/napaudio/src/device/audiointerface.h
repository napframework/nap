#pragma once

// std library includes
#include <vector>

// third party includes
#include <portaudio.h>

// nap includes
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>

// audio includes
#include <device/audioservice.h>
#include <node/audionodemanager.h>

namespace nap {
    
    namespace audio {
        
        /*
         * Represents an audio stream handling multichannel audio input and output to and from an audio device.
         * It initializes an audio callback and owns a @NodeManager that performs the processing.
         */
        class NAPAPI AudioInterface : public rtti::RTTIObject
        {
            RTTI_ENABLE(rtti::RTTIObject)
            
        public:
            AudioInterface() = default;
            
            /**
             * Constructor used by the object factory
             */
            AudioInterface(AudioService& service);
            
            /**
             * Destructor stops the audio stream 
             */
            virtual ~AudioInterface();
            
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
            NodeManager& getNodeManager() { return mNodeManager; }
            
        public:
            // PROPERTIES
            
            /** 
             * If true, the default audio input and output device on this system are being used
             */
            bool mUseDefaultDevice = false;
            
            /**
             * The number of the input device being used. Use @AudioService to poll for available devices.
             */
            int mInputDevice = 0;

            /** 
             * The number of the output device being used. Use @AudioService to poll for available devices.
             */
            int mOutputDevice = 0;
            
            /* 
             * The number of input channels in the stream. 
             * If the chosen device @mInputDevice does not support this amount of channels the stream will not start.
             */
            int mInputChannelCount = 1;

            /* 
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

        private:
            /*
             * Start the audio stream using the default audio devices available on the system.
             */
            bool startDefaultDevice(utility::ErrorState& errorState);
            
            NodeManager mNodeManager; // The node manager that performs the audio processing.
            
            PaStream* mStream = nullptr; // Pointer to the stream managed by portaudio.
            
            AudioService* mService = nullptr; // The audio service
        };
        
        using AudioInterfaceCreator = rtti::ObjectCreator<AudioInterface, AudioService>;
        
        
    }
    
}
