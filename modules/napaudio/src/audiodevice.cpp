// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>

// Audio includes
#include "audiodevice.h"

namespace nap {
    
    namespace audio {
                
        unsigned int AudioDeviceManager::getDeviceCount()
        {
            return Pa_GetDeviceCount();
        }
        
        
        const PaDeviceInfo& AudioDeviceManager::getDeviceInfo(unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(deviceIndex);
        }
        
        
        std::vector<const PaDeviceInfo*> AudioDeviceManager::getDevices()
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                result.emplace_back(&getDeviceInfo(i));
            }
            return result;
        }
        
        
        void AudioDeviceManager::printDevices()
        {
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                const PaDeviceInfo& info = *Pa_GetDeviceInfo(i);
                std::cout << i << " " << info.name << " " << info.maxInputChannels << " inputs " << info.maxOutputChannels << " outputs" << std::endl;
            }
        }
        
    }
}
