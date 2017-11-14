// Std includes
#include <iostream>

// Nap includes
#include <nap/logger.h>
#include <utility/stringutils.h>

// Audio includes
#include "audiodeviceservice.h"
#include "audiodevice.h"

RTTI_DEFINE(nap::audio::AudioDeviceService)

namespace nap
{
    
    namespace audio
    {
        
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
        
        
        unsigned int AudioDeviceService::getDeviceCount(unsigned int hostApiIndex)
        {
            return getHostApiInfo(hostApiIndex).deviceCount;
        }
        
        
        const PaDeviceInfo& AudioDeviceService::getDeviceInfo(unsigned int hostApiIndex, unsigned int deviceIndex)
        {
            return *Pa_GetDeviceInfo(Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex));
        }
        
        
        std::vector<const PaDeviceInfo*> AudioDeviceService::getDevices(unsigned int hostApiIndex)
        {
            std::vector<const PaDeviceInfo*> result;
            for (auto i = 0; i < getHostApiInfo(hostApiIndex).deviceCount; ++i)
            {
                result.emplace_back(&getDeviceInfo(hostApiIndex, i));
            }
            return result;
        }
        
        
        void AudioDeviceService::printDevices()
        {
            Logger::info("Available audio devices on this system:");
            for (auto hostApi = 0; hostApi < Pa_GetHostApiCount(); ++hostApi)
            {
                const PaHostApiInfo& hostApiInfo = *Pa_GetHostApiInfo(hostApi);
                nap::Logger::info("%s:", hostApiInfo.name);
                for (auto device = 0; device < hostApiInfo.deviceCount; ++device)
                {
                    auto index = Pa_HostApiDeviceIndexToDeviceIndex(hostApi, device);
                    const PaDeviceInfo& info = *Pa_GetDeviceInfo(index);
                    nap::Logger::info("%i: %s %i inputs %i outputs", device, info.name, info.maxInputChannels, info.maxOutputChannels);
                }
            }
        }
        
        
        std::string AudioDeviceService::getDeviceName(unsigned int hostApiIndex, unsigned int deviceIndex)
        {
            assert(hostApiIndex < getHostApiCount());
            assert(deviceIndex < getDeviceCount(hostApiIndex));
            return getDeviceInfo(hostApiIndex, deviceIndex).name;
        }
        
        
        int AudioDeviceService::getDeviceIndex(const std::string& hostApi, const std::string& device)
        {
            for (auto hostApiIndex = 0; hostApiIndex < getHostApiCount(); ++hostApiIndex)
            {
                auto hostApiInfo = getHostApiInfo(hostApiIndex);
                if (nap::utility::toLower(hostApi) == nap::utility::toLower(hostApiInfo.name))
                {
                    for (auto deviceIndex = 0; deviceIndex < hostApiInfo.deviceCount; ++deviceIndex)
                    {
                        auto deviceInfo = getDeviceInfo(hostApiIndex, deviceIndex);
                        if (nap::utility::toLower(device) == nap::utility::toLower(deviceInfo.name))
                            return Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, deviceIndex);
                    }
                }
            }
            return -1;
        }

        
    }
}

RTTI_DEFINE(nap::audio::AudioDeviceService)
