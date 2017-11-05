// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>

// Audio includes
#include "audiodeviceservice.h"
#include "audiodevice.h"

RTTI_DEFINE(nap::audio::AudioDeviceService)

namespace nap {
    
    namespace audio {
        
        AudioDeviceService::AudioDeviceService() : mInterface(*this)
        {
        }

        
        AudioDeviceService::~AudioDeviceService()
        {
            auto error = Pa_Terminate();
            if (error != paNoError)
                Logger::warn("Portaudio error: " + std::string(Pa_GetErrorText(error)));
            Logger::info("Portaudio terminated");
        }
        
        
        NodeManager& AudioDeviceService::getNodeManager()
        {
            return mInterface.getNodeManager();
        }
        
        
        bool AudioDeviceService::init(nap::utility::ErrorState& errorState)
        {
            auto error = Pa_Initialize();
            if (error != paNoError)
            {
                std::string message = "Portaudio error: " + std::string(Pa_GetErrorText(error));
                errorState.fail(message);
                return false;
            }
            
            Logger::info("Portaudio initialized.");
            printDevices();
            
            // Initialize the audio device
            return mInterface.init(errorState);
        }
        
        
        unsigned int AudioDeviceService::getHostApiCount()
        {
            return Pa_GetHostApiCount();
        }
        
        
        const PaHostApiInfo& AudioDeviceService::getHostApiInfo(unsigned int hostApiIndex)
        {
            return *Pa_GetHostApiInfo(hostApiIndex);
        }
        
        
        std::vector<const PaHostApiInfo*> AudioDeviceService::getHostApis()
        {
            std::vector<const PaHostApiInfo*> result;
            for (auto i = 0; i < Pa_GetHostApiCount(); ++i)
            {
                result.emplace_back(&getHostApiInfo(i));
            }
            return result;
        }
        
        
        std::string AudioDeviceService::getHostApiName(unsigned int hostApiIndex)
        {
            assert(hostApiIndex < getHostApiCount());
            return getHostApiInfo(hostApiIndex).name;
        }
        
        
        unsigned int AudioDeviceService::getDeviceCount()
        {
            return Pa_GetDeviceCount();
        }
        
        
        const PaDeviceInfo& AudioDeviceService::getDeviceInfo(unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(deviceIndex);
        }
        
        
        std::vector<const PaDeviceInfo*> AudioDeviceService::getDevices()
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < Pa_GetDeviceCount(); ++i)
            {
                result.emplace_back(&getDeviceInfo(i));
            }
            return result;
        }
        
        
        void AudioDeviceService::printDevices()
        {
            Logger::info("Available audio devices on this system:");
            for (auto hostApi = 0; hostApi < Pa_GetHostApiCount(); ++hostApi)
            {
                const PaHostApiInfo& hostApiInfo = *Pa_GetHostApiInfo(hostApi);
                for (auto device = 0; device < hostApiInfo.deviceCount; ++device)
                {
                    auto index = Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device);
                    const PaDeviceInfo& info = *Pa_GetDeviceInfo(index);
                    nap::Logger::info("%i: %s %s %i inputs %i outputs", index, hostApiInfo.name, info.name, info.maxInputChannels, info.maxOutputChannels);
                }
            }
        }
        
        
        std::string AudioDeviceService::getDeviceName(unsigned int deviceIndex)
        {
            assert(deviceIndex < getDeviceCount());
            return getDeviceInfo(deviceIndex).name;
        }
        
    }
}

RTTI_DEFINE(nap::audio::AudioDeviceService)
