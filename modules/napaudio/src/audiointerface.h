#pragma once

// std library includes
#include <vector>

// third party includes
#include <portaudio.h>

// nap includes
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>

// audio includes
#include "audionodemanager.h"

namespace nap {
    
    namespace audio {
        
        /*
         * Represents an audio stream handling multichannel audio input and output to and from an audio device.
         * It initializes an audio callback and owns a @NodeManager that performs the processing.
         */
        class NAPAPI AudioInterface : public rtti::RTTIObject {
            RTTI_ENABLE(rtti::RTTIObject)
            
        public:
            AudioInterface() = default;
            
            /**
             * Destructor stops the audio stream and terminates portaudio
             */
            ~AudioInterface();
            
            /** 
             * Initializes portaudio, starts the audio stream and handles possible errors
             */
            bool init(utility::ErrorState& errorState) override;
            
            /**
             * (Re)starts the audio stream and the audio processing
             */
            bool start();
            
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
            AudioNodeManager& getNodeManager() { return mNodeManager; }
            
        public:
            // PROPERTIES
            
            /** 
             * If true, the default audio input and output device on this system are being used
             */
            bool mUseDefaultDevice = false;
            
            // The number of the input device being used. Use @AudioDeviceManager to poll for available devices.
            int mInputDevice = 0;

            // The number of the output device being used. Use @AudioDeviceManager to poll for available devices.
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

            /**
             * When this is true, the app will be able to run also if the audio stream fails to start.
             */
            bool mAllowFailure = true;
            
        private:
            /**
             * Start the audio stream using the default audio devices available on the system.
             */
            bool startDefaultDevice();
            
            /**
             * The node manager that performs the audio processing.
             */
            AudioNodeManager mNodeManager;
            
            /**
             * Indicates wether portaudio has been initialized.
             */
            bool mInitialized = false;
            
            /**
             * Pointer to the stream managed by portaudio.
             */
            PaStream* mStream = nullptr;
        };                
        
    }
    
}
