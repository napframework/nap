#pragma once

// std library includes
#include <vector>

// third party includes
#include <portaudio.h>

// nap includes
#include <utility/dllexport.h>

namespace nap {
    
    namespace audio {
        
        // Forward declarations
        class AudioNodeManager;
        
        class NAPAPI AudioDeviceManager {
        public:
        public:
            AudioDeviceManager(AudioNodeManager& nodeManager);
            ~AudioDeviceManager();
            
            static unsigned int getDeviceCount();
            static const PaDeviceInfo& getDeviceInfo(unsigned int deviceIndex);
            static std::vector<const PaDeviceInfo*> getDevices();
            
            void start(int inputDevice, int outputDevice, int inputChannelCount, int outputChannelCount, float sampleRate, int bufferSize);
            void startDefaultDevice(int inputChannelCount, int outputChannelCount, float sampleRate, int bufferSize);
            void stop();
            bool isActive();
            
        private:
            PaStream* mStream = nullptr;
            AudioNodeManager& mNodeManager;
        };
        
        
    }
}
